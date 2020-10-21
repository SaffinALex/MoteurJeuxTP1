/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwidget.h"

#include <QMouseEvent>
#include <QKeyEvent>

#include <math.h>

MainWidget::MainWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    geometries(0),
    textureG(0),
    textureR(0),
    textureS(0),
    heightmap(0),
    angular_x(0),
    angular_y(0),
    angularSpeed(0)
{
}

MainWidget::MainWidget(int fps, QWidget *parent) :
    QOpenGLWidget(parent),
    geometries(0),
    textureG(0),
    textureR(0),
    textureS(0),
    heightmap(0),
    angular_x(0),
    angular_y(0),
    angularSpeed(0)
{
    camOrbital = false;
    assert(fps > 0 && (1000 / fps) > 0);

    timer.start(1000 / fps, this);
}

MainWidget::~MainWidget()
{
    // Make sure the context is current when deleting the texture
    // and the buffers.
    makeCurrent();
    delete textureG;
    delete textureR;
    delete textureS;
    delete heightmap;
    delete geometries;
    doneCurrent();
}

//! [0]
void MainWidget::mousePressEvent(QMouseEvent *e)
{
    // Save mouse press position
    mousePressPosition = QVector2D(e->localPos());
}

void MainWidget::mouseReleaseEvent(QMouseEvent *e)
{
    // Mouse release position - mouse press position
    QVector2D diff = QVector2D(e->localPos()) - mousePressPosition;

    // Rotation axis is perpendicular to the mouse position difference
    // vector
    QVector3D n = QVector3D(diff.y(), diff.x(), 0.0).normalized();

    // Accelerate angular speed relative to the length of the mouse sweep
    qreal acc = diff.length() / 100.0;

    // Calculate new rotation axis as weighted sum
    rotationAxis = (rotationAxis * angularSpeed + n * acc).normalized();

    // Increase angular speed
    angularSpeed += acc;


}

void MainWidget::wheelEvent(QWheelEvent *event)
{
 int numDegrees = event->delta() / 8;
 int numSteps = numDegrees / 15;

 if (event->orientation() == Qt::Vertical) {
     positionZ += 0.01;
     update();
 } else {
     positionZ -= 0.01;
     update();
 }
 event->accept();
}

void MainWidget::keyPressEvent(QKeyEvent *e)
{
    if(!camOrbital){
        rotate_speed = 0;
        QQuaternion yRotation;
        switch (e->key())
        {
            case Qt::Key::Key_C:
                camOrbital = true;
                angle = 0;
                update();
                break;
            case Qt::Key::Key_Right:
                positionX += 0.1;
                update();
                break;
            case Qt::Key::Key_Left:
                positionX -= 0.1;
                update();
                break;
            case Qt::Key::Key_Z:
                angular_y += 1;
                update();
                break;
            case Qt::Key::Key_S:
                angular_y -= 1;
                update();
                break;
            case Qt::Key::Key_D:
                angular_x += 1;
                update();
                break;
            case Qt::Key::Key_Q:
                angular_x -= 1;
                update();
                break;
            case Qt::Key::Key_Up:
                positionY += 0.1;
                update();
                break;
            case Qt::Key::Key_Down:
                positionY -= 0.1;
                update();
                break;
            case Qt::Key::Key_A:
                positionZ += 0.1;
                update();
                break;
            case Qt::Key::Key_E:
                positionZ -= 0.1;
                update();
                break;
        }
        QQuaternion xRotation = QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, angular_x);
        yRotation = QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, angular_y);
        rotation = xRotation+yRotation;
        update();
    }
    else{
        if(e->key() == Qt::Key::Key_C) {
            camOrbital = false;          
        }
        else if(e->key() == Qt::Key::Key_Down) {
            vadd -= 0.01;
        }
        if(e->key() == Qt::Key::Key_Up) {
            vadd += 0.01;
        }
    }
}

//! [0]

//! [1]
void MainWidget::timerEvent(QTimerEvent *)
{
    if(camOrbital) angle+=vadd;
    update();
}
//! [1]

void MainWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(0, 0, 0, 1);

    initShaders();
    initTextures();

//! [2]
    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);

    // Enable back face culling
    glEnable(GL_CULL_FACE);
//! [2]

    geometries = new GeometryEngine;

    // Use QBasicTimer because its faster than QTimer
    //timer.start(12, this);
}

//! [3]
void MainWidget::initShaders()
{
    // Compile vertex shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl"))
        close();

    // Compile fragment shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl"))
        close();

    // Link shader pipeline
    if (!program.link())
        close();

    // Bind shader pipeline for use
    if (!program.bind())
        close();
}
//! [3]

//! [4]
void MainWidget::initTextures()
{
    // Load grass.png image
    textureG = new QOpenGLTexture(QImage(":/grass.png").mirrored());

    // Set nearest filtering mode for texture minification
    textureG->setMinificationFilter(QOpenGLTexture::Nearest);

    // Set bilinear filtering mode for texture magnification
    textureG->setMagnificationFilter(QOpenGLTexture::Linear);

    // Wrap texture coordinates by repeating
    // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
    textureG->setWrapMode(QOpenGLTexture::Repeat);

    // Load rock.png image
    textureR = new QOpenGLTexture(QImage(":/rock.png").mirrored());

    // Set nearest filtering mode for texture minification
    textureR->setMinificationFilter(QOpenGLTexture::Nearest);

    // Set bilinear filtering mode for texture magnification
    textureR->setMagnificationFilter(QOpenGLTexture::Linear);

    // Wrap texture coordinates by repeating
    // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
    textureR->setWrapMode(QOpenGLTexture::Repeat);

    // Load snowrocks.png image
    textureS = new QOpenGLTexture(QImage(":/snowrocks.png").mirrored());

    // Set nearest filtering mode for texture minification
    textureS->setMinificationFilter(QOpenGLTexture::Nearest);

    // Set bilinear filtering mode for texture magnification
    textureS->setMagnificationFilter(QOpenGLTexture::Linear);

    // Wrap texture coordinates by repeating
    // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
    textureS->setWrapMode(QOpenGLTexture::Repeat);

    // Load snowrocks.png image
    heightmap = new QOpenGLTexture(QImage(":/heightmap-1024x1024.png").mirrored());

    // Set nearest filtering mode for texture minification
    heightmap->setMinificationFilter(QOpenGLTexture::Nearest);

    // Set bilinear filtering mode for texture magnification
    heightmap->setMagnificationFilter(QOpenGLTexture::Linear);

    // Wrap texture coordinates by repeating
    // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
    heightmap->setWrapMode(QOpenGLTexture::Repeat);
}
//! [4]

//! [5]
void MainWidget::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Set near plane to 3.0, far plane to 7.0, field of view 45 degrees
    //const qreal zNear = 3.0, zFar = 7.0, fov = 45.0;
    const qreal zNear = 0.01, zFar = 1000.0, fov = 45.0;

    // Reset projection
    projection.setToIdentity();

    // Set perspective projection
    projection.perspective(fov, aspect, zNear, zFar);
}
//! [5]

void MainWidget::paintGL()
{
    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    textureG->bind(0);
    textureR->bind(1);
    textureS->bind(2);
    heightmap->bind(3);

//! [6]
    // Calculate model view transformation
    QMatrix4x4 matrix;
    if(!camOrbital){
        matrix.translate(positionX, positionY, positionZ);
        matrix.rotate(rotation);
    }
    else{
        //Convertis angle en radian
        float angleR = angleCam*M_PI/180;
        //Calcul hauteur camera
        rayonOrbit = 7;
        angleR = 45;
        float posCamZ = rayonOrbit/tan(angleR);
        matrix.lookAt(QVector3D(rayonOrbit*sin(angle),rayonOrbit*cos(angle),posCamZ), QVector3D(1,1,0), QVector3D(0,0,1));
    }

    // Set modelview-projection matrix
    program.setUniformValue("mvp_matrix", projection * matrix);
//! [6]

    // Use texture unit 0 which contains cube.png
    program.setUniformValue("textureG", 0);
    program.setUniformValue("textureR", 1);
    program.setUniformValue("textureS", 2);
    program.setUniformValue("heigtmap", 3);

    // Draw cube geometry
    geometries->drawCubeGeometry(&program);
}
