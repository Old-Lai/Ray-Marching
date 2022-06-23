#pragma once

#include "ofMain.h"
#include <glm/gtx/intersect.hpp>
#include "glm/gtx/euler_angles.hpp"
#include "ofxGui.h"

//  General Purpose Ray class 
//
class Ray {
public:
	Ray(glm::vec3 p, glm::vec3 d) { this->p = p; this->d = d; }
	void draw(float t) { ofDrawLine(p, p + t * d); }

	glm::vec3 evalPoint(float t) {
		return (p + t * d);
	}

	glm::vec3 p, d;
};

//  Base class for any renderable object in the scene
//
class SceneObject {
public:
	virtual void draw() = 0;    // pure virtual funcs - must be overloaded
	virtual bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) { cout << "SceneObject::intersect" << endl; return false; }

	// any data common to all scene objects goes here
	glm::vec3 position = glm::vec3(0, 0, 0);

	// material properties (we will ultimately replace this with a Material class - TBD)
	//
	ofColor diffuseColor = ofColor::grey;    // default colors - can be changed.
	ofColor specularColor = ofColor::lightGray;

	//rayMarching stuff
	virtual float sdf(const glm::vec3& p) { cout << "SceneObject::sdf" << endl; return 0; }

};

//  General purpose sphere  (assume parametric)
//
class Sphere : public SceneObject {
public:
	Sphere(glm::vec3 p, float r, ofColor diffuse = ofColor::lightGray) { position = p; radius = r; diffuseColor = diffuse; }
	Sphere() {}
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) {
		return (glm::intersectRaySphere(ray.p, ray.d, position, radius, point, normal));
	}
	void draw() {
		ofDrawSphere(position, radius);
	}

	float radius = 1.0;


	//rayMarching stuff
	float sdf(const glm::vec3& p) {
		return glm::length(position - p) - radius;
	}
};

//  Mesh class (will complete later- this will be a refinement of Mesh from Project 1)
//
class Mesh : public SceneObject {
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) { return false; }
	void draw() { }
};

//  General purpose plane 
//
class Plane : public SceneObject {
public:
	Plane(glm::vec3 p, glm::vec3 n, ofColor diffuse = ofColor::darkOliveGreen, float w = 20, float h = 20) {
		position = p; normal = n;
		width = w;
		height = h;
		diffuseColor = diffuse;
		plane.rotateDeg(90, 1, 0, 0);
	}
	Plane() { }
	glm::vec3 normal = glm::vec3(0, 1, 0);
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal);
	void draw() {
		plane.setPosition(position);
		plane.setWidth(width);
		plane.setHeight(height);
		plane.setResolution(4, 4);
		plane.drawWireframe();
	}
	ofPlanePrimitive plane;
	float width = 20;
	float height = 20;

	//RayMarching stuff
	float sdf(const glm::vec3& p)
	{
		return p.y - plane.getY();
	}
};

//Torus
//
class Torus : public SceneObject {
public:
	Torus(glm::vec3 p, glm::vec2 t, ofColor diffuse = ofColor::lightGray, glm::vec3 rot = glm::vec3(20,45,0)) {
		position = p;
		this->t = t;
		diffuseColor = diffuse;
		rotation = rot;
	}
	Torus() {};
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) {
		return false;
	}
	void draw() {
		ofDrawSphere(position, 1);
	}

	glm::vec2 t = glm::vec2(1, 2);
	glm::vec3 rotation;

	glm::mat4 getRotateMatrix() {
		return (glm::eulerAngleXYZ(glm::radians(rotation.y), glm::radians(rotation.x), glm::radians(rotation.z)));   // yaw, pitch, roll 
	}
	glm::mat4 getTranslateMatrix() {
		return (glm::translate(glm::mat4(1.0), glm::vec3(position.x, position.y, position.z)));
	}

	//rayMarching stuff
	float sdf(const glm::vec3& p) {
		glm::vec4 temp = glm::inverse(getTranslateMatrix()) * glm::inverse(getRotateMatrix()) * glm::vec4(p, 1);
		
		glm::vec2 q = glm::vec2(glm::length(glm::vec2(temp.x, temp.y)) - t.x, temp.z);
		return glm::length(q) - t.y;
	}
};

//HollowSphere
//
class HollowSphere : public SceneObject {
public:
	HollowSphere(glm::vec3 p, ofColor diffuse = ofColor::lightGray, glm::vec3 rot = glm::vec3(20, -45, 0)) {
		position = p;
		this->rht = rht;
		diffuseColor = diffuse;
		rotation = rot;
	}
	HollowSphere() {};
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) {
		return false;
	}
	void draw() {
		ofDrawSphere(position, 1);
	}

	glm::vec3 rht = glm::vec3(0.1, 0.05, 0.05);
	glm::vec3 rotation;

	glm::mat4 getRotateMatrix() {
		return (glm::eulerAngleXYZ(glm::radians(rotation.y), glm::radians(rotation.x), glm::radians(rotation.z)));   // yaw, pitch, roll 
	}
	glm::mat4 getTranslateMatrix() {
		return (glm::translate(glm::mat4(1.0), glm::vec3(position.x, position.y, position.z)));
	}

	//rayMarching stuff
	float sdf(const glm::vec3& p) {
		glm::vec4 temp = glm::inverse(getRotateMatrix()) * glm::inverse(getTranslateMatrix()) * glm::vec4(p, 1);

		glm::vec2 q = glm::vec2(glm::length(glm::vec2(temp.x, temp.y)), temp.z);

		float w = sqrt(rht.x * rht.x - rht.y * rht.y);
		
		return ((rht.y * q.x < w* q.y) ? length(q - glm::vec2(w, rht.y)) : abs(glm::length(q) - rht.x)) - rht.z;
	}
};

// view plane for render camera
// 
class  ViewPlane : public Plane {
public:
	ViewPlane(glm::vec2 p0, glm::vec2 p1) { min = p0; max = p1; }

	ViewPlane() {                         // create reasonable defaults (6x4 aspect)
		min = glm::vec2(-3, -2);
		max = glm::vec2(3, 2);
		position = glm::vec3(0, 0, 5);
		normal = glm::vec3(0, 0, 1);      // viewplane currently limited to Z axis orientation
	}

	void setSize(glm::vec2 min, glm::vec2 max) { this->min = min; this->max = max; }
	float getAspect() { return width() / height(); }

	glm::vec3 toWorld(float u, float v);   //   (u, v) --> (x, y, z) [ world space ]

	void draw() {
		ofDrawRectangle(glm::vec3(min.x, min.y, position.z), width(), height());
	}


	float width() {
		return (max.x - min.x);
	}
	float height() {
		return (max.y - min.y);
	}

	// some convenience methods for returning the corners
	//
	glm::vec2 topLeft() { return glm::vec2(min.x, max.y); }
	glm::vec2 topRight() { return max; }
	glm::vec2 bottomLeft() { return min; }
	glm::vec2 bottomRight() { return glm::vec2(max.x, min.y); }

	//  To define an infinite plane, we just need a point and normal.
	//  The ViewPlane is a finite plane so we need to define the boundaries.
	//  We will define this in terms of min, max  in 2D.  
	//  (in local 2D space of the plane)
	//  ultimately, will want to locate the ViewPlane with RenderCam anywhere
	//  in the scene, so it is easier to define the View rectangle in a local'
	//  coordinate system.
	//
	glm::vec2 min, max;
};


//  render camera  - currently must be z axis aligned (we will improve this in project 4)
//
class RenderCam : public SceneObject {
public:
	RenderCam() {
		position = glm::vec3(0, 0, 10);
		aim = glm::vec3(0, 0, -1);
	}
	Ray getRay(float u, float v);
	void draw() { ofDrawBox(position, 1.0); };
	void drawFrustum();

	glm::vec3 aim;
	ViewPlane view;          // The camera viewplane, this is the view that we will render 
};

class Light : public SceneObject {
public:
	float intensity = 1;
	Light(glm::vec3 p, float intensity = 1, ofColor diffuse = ofColor::white) {
		position = p;
		this->intensity = intensity;
		diffuseColor = diffuse;
	}
	Light() {}

	void draw() { ofDrawSphere(position, 0.1); }
};



class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	void rayTrace();
	void drawGrid();
	void drawAxis(glm::vec3 position);
	bool rayMarching(Ray r, glm::vec3& p);
	void rayMarchLoop();
	float opRep(const glm::vec3& p, SceneObject* obj);

	glm::vec3 getNormalRM(const glm::vec3& p);
	float sceneSDF(const glm::vec3 p, int& objIndex); 

	void progressBar(float progress, int& prevPos);

	ofColor lambert(const glm::vec3& p, const glm::vec3& n, const ofColor diffuse);
	ofColor phong(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse,
		const ofColor specular, float power);
	bool inShadow(Ray r);

	bool bHide = true;
	bool bShowImage = false;

	ofEasyCam  mainCam;
	ofCamera sideCam;
	ofCamera previewCam;
	ofCamera* theCam;    // set to current camera either mainCam or sideCam

	// set up one render camera to render image throughn
	//
	RenderCam renderCam;
	ofImage image;

	vector<SceneObject*> scene;

	int imageWidth = 1200;
	int imageHeight = 800;

	vector<Light> lights;

	ofxPanel gui;
	ofxIntSlider powerSlider;
	ofxFloatSlider ambientLightSlider, lightIntensitySlider1,
		lightIntensitySlider2, lightIntensitySlider3;
};

