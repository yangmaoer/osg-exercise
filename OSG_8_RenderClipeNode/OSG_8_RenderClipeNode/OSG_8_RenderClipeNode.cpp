// OSG_8_RenderClipeNode.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"


/*
裁剪模型
该方法并不是真正的裁剪，它只是不显示“被裁减”区域

创建裁剪节点，设置包围盒和裁剪平面
虽然设置了范围，但实际上这个节点什么也没有绘制，当有什么模型添加到该节点中，这个节点就裁剪什么模型，包围盒内显示，盒外不显示

创建更新回调节点和空间变换节点，空间变换节点调用更新回调节点的更新回调方法，并将裁剪节点添加到空间变换矩阵节点中
也即是clipnode节点截取的是什么模型，此处旋转的就是什么模型

创建未被裁剪的节点，获取裁剪节点的渲染状态，并添加模型节点
因此该节点也有了包围盒和裁剪平面，它添加了什么模型节点，它就裁剪什么模型

创建组节点为多边形模型节点，并设置其渲染状态为多边形线形模型，并添加物体模型
则这个物体模型是线形的

创建组结点为根节点，将空间变换节点、未被裁剪节点和线形模型节点添加到其中
*/
osg::ref_ptr<osg::Node> createClipNode(osg::ref_ptr<osg::Node> subgraph)
{
	//获取中心点和半径 osg::BoundingSphere用于封闭节点/对象/顶点的通用边界球类 
	osg::BoundingSphere bs = subgraph->getBound();
	//设置半径长度为原来的0.4倍
	bs.radius() *= 0.4f;
	//设置裁剪节点的包围盒，包围盒的长度为改变后的半径值，盒以外的都将裁剪掉
	osg::BoundingBox bb;
	bb.expandBy(bs);
	//创建裁剪节点
	osg::ref_ptr<osg::ClipNode> clipnode = new osg::ClipNode();
	//根据前面指定的包围盒创建6个裁剪平面
	clipnode->createClipBox(bb);
	//禁用拣选（拣选即显示一些东西不显示一些东西）
	clipnode->setCullingActive(false);
	//释放下面这句话，然后将root添加的节点clippedNode改为clipnode，另外两个不变
	//出现效果为静止的线形完整模型、静止的填充裁剪模型、转动的填充裁剪模型
	//这是由于它里面的裁剪节点、飞机节点等都属于在原位置对该节点进行深拷贝，它们并不是原始的节点
	//clipnode->addChild(subgraph.get());

	//更新回调，实现动态裁剪，需要将更新回调类和矩阵变换节点类相结合，更新回调类改变当前节点，矩阵变换类改变节点位置
	//创建一个动画路径来产生关于一个点的旋转 参数1坐标原点const osg::Vec3d &pivot 参数2坐标轴const osg::Vec3d &axis 参数3旋转角度float angularVelocity)
	//getBound().center() 获取节点的通用边界球的中心点，osg::inDegrees(45.0f)将45角度转换为弧度
	osg::ref_ptr<osg::NodeCallback> nc = new osg::AnimationPathCallback(subgraph->getBound().center(), osg::Vec3(0.0f, 0.0f, 1.0f), osg::inDegrees(45.0f));
	//创建空间变换矩阵节点对象MatrixTransform
	osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform();
	transform->setUpdateCallback(nc.get());
	transform->addChild(clipnode.get());
	
	//创建未被裁剪的节点
	osg::ref_ptr<osg::Group> clippedNode = new osg::Group();
	clippedNode->setStateSet(clipnode->getStateSet());
	clippedNode->addChild(subgraph.get());

	//多边形模型绘制，绘制面为正面和反面，绘制模式为线形
	osg::ref_ptr<osg::PolygonMode> polynmode = new osg::PolygonMode();
	polynmode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
	//实例化一个渲染状态
	osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet();
	//启用与上面相对应的多边形线形绘制模式，并应用将上面设置的模式
	//制定状态继承属性为OVERRIDE（所有子节点都将继承这一属性或模式，子节点对其更改无效）
	stateset->setAttributeAndModes(polynmode, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);

	//多边形线形绘制节点
	osg::ref_ptr<osg::Group> wireframe_subgraph = new osg::Group();
	//设置渲染状态
	wireframe_subgraph->setStateSet(stateset.get());
	wireframe_subgraph->addChild(subgraph.get());

	/*
	注释掉后两句，只添加空间变换矩阵节点，不显示模型，因为此时没有基础模型存在
	注释掉前后两句，只添加未被裁剪的节点，显示填充完的模型（原始飞机模型）
	注释掉前两句，只添加多边形线形绘制节点，显示线形模型

	注释掉第一句，不添加空间变换矩阵节点，显示填充完的模型（原始飞机模型）
	注释掉第二句，不添加未被裁剪的节点，显示线形模型
	注释掉最后一句，不添加多边形线形绘制节点，在填充模型（原始飞机模型）基础上进行动态裁剪	

	不注释任何语句，在填充模型（原始飞机模型）基础上进行动态裁剪，裁剪部分显示线形模型
	非裁剪节点获取了裁剪节点的渲染状态，但动态更新这个方法并不能一起获取
	但是裁剪节点的动态更新方法使得其包围盒变成了一个动态的包围盒，包围盒是一种渲染状态，而非裁剪节点的包围盒与其一致，因此也是动态更新的，则出现了这种动态裁剪

	将空间变换矩阵节点transform改为裁剪节点clipnode，显示静态的裁剪模型，非裁剪部分显示填充模型（原始飞机模型），裁剪部分显示线形模型
	*/
	osg::ref_ptr<osg::Group> root = new osg::Group();
	//添加空间变换矩阵节点
	root->addChild(transform.get());
	//添加未被裁剪的节点
	root->addChild(clippedNode.get());
	//添加多边形线形绘制节点
	root->addChild(wireframe_subgraph.get());

	return root.release();
}
int main()
{  
	//加载模型
	osg::ref_ptr<osg::Node> node = osgDB::readNodeFile("cessna.osg");
	osg::ref_ptr<osg::Node> root = createClipNode(node.get());

	//优化场景数据  
	osgUtil::Optimizer optimizer;
	optimizer.optimize(root.get());

	//显示模型  
	osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer();
	viewer->setSceneData(root.get());
	viewer->realize();

	return viewer->run();
}

