// OSG_5_GeometryRotatingLine.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"


/*从几何体的更新回调类UpdateCallback派生用户类，并重构执行函数update()，实现自定义的更新回调功能
设置可绘制体的更新回调DynamicLineCallback
执行函数获取几何体顶点数组的地址
修改各个顶点的值，并使用dirty()函数将修改结果通知给VBO对象
使用Drawable::setUpdateCallback(new 自定义类)将自定义类设置给几何体后回调函数将在系统运行的每一帧被调用
*/
class DynamicLineCallback :public osg::Drawable::UpdateCallback
{
	float _angle = 0.0;

	//void update(NodeVisitor *, Drawable *)虚函数，重构后可以实时获取和操作几何体
	//参数：系统内置的访问器对象，用于遍历节点树并找到回调对象的位置；回调所在几何体的对象指针，以便对其进行操作
	virtual void update(osg::NodeVisitor *nv, osg::Drawable *drawable)
	{
		//dynamic_cast <type-id> (expression):把expression转换成type-id类型的对象, type-id必须是类的指针、类的引用或者void *, expression要与Type-id相同
		//dynamic_cast运算符将一个基类对象指针(或引用)cast到继承类指针，dynamic_cast会根据基类指针是否真正指向继承类指针来做相应处理, 它可以在执行期决定真正的类型
		//dynamic_cast主要用于类层次间的上行转换(和static_cast效果一样)和下行转换(具有类型检查的功能，比static_cast更安全)，还可以用于类之间的交叉转换
		osg::Geometry *geom = dynamic_cast<osg::Geometry *>(drawable);
		if (!geom)
			return;

		/*手动更新：获取数组
		然后在循环中使迭代器依次指向容器中的下一个元素（可以将迭代器看成一个指针，但实际上C++中指针是一种迭代器，迭代器不仅仅是指针），将下一个元素值赋给上一个元素
		*/

		//获取顶点数组，vertices是一个数组指针，即是一个指向顶点数组的指针
		osg::Vec3Array *vertices = dynamic_cast<osg::Vec3Array *>(geom->getVertexArray());
		if (vertices)
		{
			//定义一个名为itr的迭代器类型变量
			//c.begin() 返回一个当前容器中起始元素的迭代器，即使迭代器itr指向容器的第一个元素
			//c.end() 返回一个当前容器中末尾元素的下一个位置的迭代器，称为超出末端迭代器，即-1后使迭代器itr指向容器的最后一个元素
			for (osg::Vec3Array::iterator itr = vertices->begin(); itr != vertices->end() - 1; ++itr)
				//*itr 对itr进行解引用，返回迭代器itr指向的元素的引用，可以理解为读取该元素的值，我认为可以理解为快捷方式
				//iter->set(a) osg自定义函数，设置itr指向的元素的引用值为a，改变引用则改变原来的值（快捷方式的性质），因此即改变顶点数组中对应的值
				itr->set(*(itr + 1));

			//循环结束后，顶点数组中第1个元素到倒数第2个元素的值都变成了它们下一位的元素值，则数组中最后两个元素值相同，因此下面要进行的操作是为最后一个元素赋值
			//最后一个元素的值是一个新值，线段向前移动的那个值

			_angle += 1.0 / 10.0;

			//c.front()和c.back() 返回容器的首尾元素的引用
			//c.push_back(v) 向量专用，将元素v加入到c容器的最后一位，使其长度增加一位，可以理解为压栈（但实际上有区别）
			osg::Vec3 &pt = vertices->back();
			pt.set(10.0*cos(_angle), 0.0, 10.0*sin(_angle));

			//使用dirty()将修改结果通知给VBO对象
			vertices->dirty();
		}
	}
};

//创建一个简单的线对象
osg::Drawable *createLine()
{
	//创建顶点数组
	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(10);
	//添加数据
	for (unsigned int i = 0; i < 10; ++i)
		(*vertices)[i].set(float(i), 0.0, 0.0);

	//创建一个几何体对象  
	osg::ref_ptr<osg::Geometry> lineGeom = new osg::Geometry();
	//设置顶点数据
	lineGeom->setVertexArray(vertices.get());
	//添加图元，多段直线条带：第一段由数组中的前两个点决定，其余段的起点位置为上一段的终点坐标，而终点位置由数组中随后的点决定
	lineGeom->addPrimitiveSet(new osg::DrawArrays(osg::DrawArrays::LINE_STRIP, 0, 10));

	//使用setInitialBound()设置其初始包围盒，以免在运动中包围体的不断变化导致场景包围体层次不明确
	lineGeom->setInitialBound(osg::BoundingBox(osg::Vec3(-10.0, -10.0, -10.0), osg::Vec3(10.0, 10.0, 10.0)));

	//使用setUpdateCallback()将自定义类设置给几何体后回调函数将在系统运行的每一帧被调用
	lineGeom->setUpdateCallback(new DynamicLineCallback);

	//设置setUseDisplayList()或者setUseVert exBufferObjects()渲染几何体
	//设置setUseDisplayList()开启顶点数组来渲染几何体，但需要实时调用dirtyDisplayList()来强制刷新与其关联的显示列表（显示列表默认开启，且只在构建时被执行一次）
	//为了避免这种每帧都执行一次显示列表的重建工作，则选择慢速通道（低配系统使用）或者VBO的数据渲染方式setUseVertexBufferObjects()
	lineGeom->setUseDisplayList(false);
	lineGeom->setUseVertexBufferObjects(true);

	return lineGeom.release();
}

int main()
{
	//创建一个叶节点对象
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;

	//使用渲染属性和模式适当地调整显示效果
	geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	geode->getOrCreateStateSet()->setAttribute(new osg::LineWidth(2.0));

	//将线添加到叶节点
	geode->addDrawable(createLine());

	osgViewer::Viewer viewer;
	viewer.setSceneData(geode.get());

	return viewer.run();
}