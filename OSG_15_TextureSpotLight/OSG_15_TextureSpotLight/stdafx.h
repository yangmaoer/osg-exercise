// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>



// TODO: 在此处引用程序需要的其他头文件
#include <osg/Node>
#include <osg/Geode>  
#include <osg/Geometry>
#include <osg/Group>

#include <osg/Camera>

#include <osg/MatrixTransform> //移动节点的矩阵类，最常用的移动节点的类。可随动、旋转控制节点。
#include <osg/PositionAttitudeTransform>

#include <osg/Light> //继承自osg::StateAttribute，保存灯光的模式与属性参数信息
#include <osg/LightSource> //继承自osg::Group，灯光管理类，继承了osg::Group类的管理节点的接口，将灯光作为一个节点加入到场景图中进行渲染

#include <osg/ShapeDrawable>
#include <osg/TexGen>
#include <osg/Texture2D>
#include <osg/TexGenNode>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgUtil/Optimizer>
#include <osgViewer/Viewer>