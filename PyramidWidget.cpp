#include "PyramidWidget.h"
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>

// Конструктор класса PyramidWidget
PyramidWidget::PyramidWidget(QWidget *parent)
    : QOpenGLWidget(parent), x0(50), y0(20) // Инициализация начальных значений
{
    // Подключение таймера к слоту обновления вида
    connect(&timer, &QTimer::timeout, this, &PyramidWidget::updateView);
    timer.start(100);
}

// Деструктор класса PyramidWidget
PyramidWidget::~PyramidWidget()
{
}

// Инициализация OpenGL
void PyramidWidget::initializeGL()
{
    initializeOpenGLFunctions(); // Инициализация функций OpenGL
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Установка черного фона
    glEnable(GL_DEPTH_TEST); // Включение теста глубины для правильного отображения 3D объектов
}

// Обработка изменения размера окна
void PyramidWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h); // Установка области просмотра OpenGL
}

// Рисование сцены
void PyramidWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Очистка буферов цвета и глубины

    // Настройка проекционной матрицы
    QMatrix4x4 projection;
    projection.perspective(45.0f, float(width()) / height(), 0.1f, 100.0f); // Установка перспективной проекции

    // Настройка вида (камера)
    // Камера находится в точке (x0, y0, 50), смотрит на точку (0, 0, 0), верхний вектор направлен вдоль оси z
    projection.lookAt(QVector3D(x0, y0, 50), QVector3D(0, 0, 0), QVector3D(0, 0, 1));

    // Настройка модели
    QMatrix4x4 modelView;

    // Вычисление матрицы модели-проекции
    QMatrix4x4 mvp = projection * modelView;

    // Настройка шейдерной программы
    QOpenGLShaderProgram program;

    // Добавление вершинного шейдера
    program.addShaderFromSourceCode(QOpenGLShader::Vertex, R"(
    attribute vec3 vertexPosition;              // Атрибут для позиции вершины
    attribute vec3 vertexColor;                 // Атрибут для цвета вершины
    varying vec3 fragColor;                     // Переменная для передачи цвета во фрагментный шейдер
    uniform mat4 mvpMatrix;                     // Матрица модели-вида-проекции
    void main() {
        fragColor = vertexColor;                // Передача цвета вершины
        gl_Position = mvpMatrix * vec4(vertexPosition, 1.0); // Вычисление итоговой позиции вершины
    }
    )");

    // Добавление фрагментного шейдера
    program.addShaderFromSourceCode(QOpenGLShader::Fragment, R"(
    varying vec3 fragColor;                     // Переменная для приема цвета из вершинного шейдера
    void main() {
        gl_FragColor = vec4(fragColor, 1.0);    // Установка цвета пикселя на основе цвета вершины
    }
    )");

    program.link(); // Компиляция и связывание шейдеров
    program.bind(); // Активация шейдерной программы

    program.setUniformValue("mvpMatrix", mvp); // Передача матрицы модели-проекции в шейдер

    // Определение вершин пирамиды и их цветов
    GLfloat Ver[] = {
        // Координаты вершин и цвета
        -10.0f, -5.0f, 0.0f,    1.0f, 0.0f, 0.0f, // Нижняя левая вершина основания (красный)
        10.0f, -5.0f, 0.0f,     0.0f, 1.0f, 0.0f, // Нижняя правая вершина основания (зеленый)
        10.0f,  5.0f, 0.0f,     0.0f, 0.0f, 1.0f, // Верхняя правая вершина основания (синий)
        -10.0f,  5.0f, 0.0f,    1.0f, 1.0f, 0.0f, // Верхняя левая вершина основания (желтый)
        0.0f,  0.0f, 20.0f,     1.0f, 0.0f, 1.0f  // Вершина пирамиды (фиолетовый)
    };

    // Определение индексов вершин для отрисовки граней пирамиды линиями
    GLuint Reb[] = {
        0, 1, // Основание
        1, 2,
        2, 3,
        3, 0,
        0, 4, // Боковые грани
        1, 4,
        2, 4,
        3, 4
    };

    // Активация и настройка атрибутов вершин
    program.enableAttributeArray("vertexPosition");
    program.setAttributeArray("vertexPosition", GL_FLOAT, Ver, 3, 6 * sizeof(GLfloat));

    program.enableAttributeArray("vertexColor");
    program.setAttributeArray("vertexColor", GL_FLOAT, Ver + 3, 3, 6 * sizeof(GLfloat));

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Установка режима wireframe (отрисовка только линий а не полигонов)
    glDrawElements(GL_LINES, 16, GL_UNSIGNED_INT, Reb); // Отрисовка пирамиды линиями

    // Деактивация атрибутов вершин
    program.disableAttributeArray("vertexPosition");
    program.disableAttributeArray("vertexColor");
    program.release(); // Деактивация шейдерной программы
}

// Обновление точки зрения
void PyramidWidget::updateView()
{
    if (y0 == 20) x0 -= 5;
    if (x0 == -50) y0 -= 5;
    if (y0 == -20) x0 += 5;
    if (x0 == 50) y0 += 5;
    update(); // Перерисовка сцены
}
