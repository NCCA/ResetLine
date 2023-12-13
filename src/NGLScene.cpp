#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/VAOFactory.h>
#include <ngl/SimpleIndexVAO.h>
#include <ngl/Random.h>
#include <iostream>
#include <ngl/Util.h>
#include <ngl/NGLStream.h>
#include <ngl/ShaderLib.h>
#include <limits>


NGLScene::NGLScene()
{
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  setTitle("Restart Line");
  std::uniform_int_distribution<> numLines(2,12);
  ngl::Random::addIntGenerator("numLines",numLines);
}


NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
}



void NGLScene::resizeGL(int _w , int _h)
{
  m_win.width  = static_cast<int>( _w * devicePixelRatio() );
  m_win.height = static_cast<int>( _h * devicePixelRatio() );
  m_project=ngl::perspective(45.0f,float(_w)/float(_h),0.001f,100.0f);

}


void NGLScene::initializeGL()
{
  // we must call that first before any other GL commands to load and link the
  // gl commands from the lib, if that is not done program will crash
  ngl::NGLInit::initialize();
  glClearColor(0.7f, 0.7f, 0.7f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
  m_vao=ngl::VAOFactory::createVAO(ngl::simpleIndexVAO,GL_LINE_STRIP);
  glPointSize(4);
  createBlades();
  m_view=ngl::lookAt({0,10,10},{0,0,0},{0,1,0});
  m_project=ngl::perspective(45.0f,1.0f,0.001f,100.0f);

  ngl::ShaderLib::loadShader("LineShader","shaders/LineVertex.glsl","shaders/LineFragment.glsl");
  ngl::ShaderLib::use("LineShader");
  startTimer(10);

}


void NGLScene::createBlades()
{
    m_blades.resize(m_cols*m_rows);
    float zStep=static_cast<float>(m_colSize)/m_cols;
    float xStep=static_cast<float>(m_rowSize)/m_rows;
    size_t index=0;
    float xpos=-(m_colSize*0.5f);
    float zpos=-(m_rowSize*0.5f);

    for(int z=0; z<m_cols; ++z)
    {
        for(int x=0; x<m_rows; ++x)
        {
            m_blades[index]=createLines(ngl::Vec3(xpos,0.0f,zpos));
            ++index;
            xpos+=xStep;
        }
        xpos=-(m_colSize*0.5f);
        zpos+=zStep;
    }
}

std::vector<ngl::Vec3> NGLScene::createLines(ngl::Vec3 &_pos)
{
    std::vector<ngl::Vec3> points;
    // store first point
    points.push_back(_pos);

    auto numLines = ngl::Random::getIntFromGeneratorName("numLines");
    auto height= 1.0f+ngl::Random::randomPositiveNumber(1.5f);
    float step=height/static_cast<float>(numLines);
    auto pos=_pos;
    for(int i=0; i<numLines; ++i)
    {
        pos.m_y +=step;
        pos.m_x += ngl::Random::randomNumber(0.1f);
        pos.m_z += ngl::Random::randomNumber(0.1f);
        points.push_back(pos);
    }
    return points;
}



void NGLScene::renderVAO()
{
    std::vector<ngl::Vec3> points;
    std::vector<GLuint> index;
    GLuint idx=0;
    GLuint restart=std::numeric_limits<GLuint>::max()-1;
    ngl::Vec3 baseColour(0.1f,0.2f,0.1f);
    ngl::Vec3 tipColour(0.0f,1.0f,0.0f);
    for(auto b : m_blades)
    {
        float t=0.0f;
        float tStep=1.0f/b.size();
        for (auto p: b)
        {
            points.push_back(p);
            points.push_back(ngl::lerp(baseColour,tipColour,t));
            t+=tStep;
            index.push_back((idx++));
        }
        index.push_back(restart);
    }
    m_vao->bind();
    glPrimitiveRestartIndex(restart);
    glEnable(GL_PRIMITIVE_RESTART);

    m_vao->setData(ngl::SimpleIndexVAO::VertexData(
            points.size()*sizeof(ngl::Vec3),
            points[0].m_x,
            index.size(), &index[0],
            GL_UNSIGNED_INT));
    // points
    m_vao->setVertexAttributePointer(0, 3, GL_FLOAT, sizeof(ngl::Vec3)*2, 0);
    // colours
    m_vao->setVertexAttributePointer(1, 3, GL_FLOAT, sizeof(ngl::Vec3)*2, 3);
    m_vao->setNumIndices(points.size()/2);

    m_vao->draw();
    m_vao->unbind();
    glDisable(GL_PRIMITIVE_RESTART);

}



void NGLScene::paintGL()
{
    // clear the screen and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0,0,m_win.width,m_win.height);
    // Rotation based on the mouse position for our global transform
    auto rotX = ngl::Mat4::rotateX(m_win.spinXFace);
    auto rotY = ngl::Mat4::rotateY(m_win.spinYFace);

    // multiply the rotations
    auto mouseGlobalTX = rotX * rotY;
    // add the translations
    mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
    mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
    mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;
    ngl::ShaderLib::use("LineShader");
    ngl::ShaderLib::setUniform("MVP",m_project*m_view*mouseGlobalTX);
    renderVAO();
}

//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
  case Qt::Key_Space :
      m_win.spinXFace=0;
      m_win.spinYFace=0;
      m_modelPos.set(ngl::Vec3::zero());

  break;
  case Qt::Key_A : m_animate^=true; break;
  default : break;
  }
  // finally update the GLWindow and re-draw

    update();
}

void NGLScene::timerEvent(QTimerEvent *_event)
{
    if(!m_animate)
        return;
    static float t=0.0f;

    for(auto &p : m_blades)
    {
        for(size_t i=1; i<p.size(); ++i)
        {
            p[i].m_x+=sinf(t*p[i].m_y)*0.001f;
            p[i].m_z+=cosf(t*p[i].m_y)*0.001f;
        }
    }

    t+=0.05f;
    update();
}