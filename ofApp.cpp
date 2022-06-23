#include "ofApp.h"
/*
 This program takes in hard programed in scene objects and generate a ray traced
 image from a prefixed camera along the z axis. this will save the image as
 Output.png. F4 key can be used to preview it in the program window. F3 to render.
 Now updated with Lambert and Phong shading. Sliders available to adjust settings.
 */

 // Intersect Ray with Plane  (wrapper on glm::intersect*
 //
bool Plane::intersect(const Ray& ray, glm::vec3& point, glm::vec3& normalAtIntersect) {
    float dist;
    bool hit = glm::intersectRayPlane(ray.p, ray.d, position, this->normal, dist);
    if (hit) {
        Ray r = ray;
        point = r.evalPoint(dist);
        normalAtIntersect = this->normal;
    }
    return (hit);
}


// Convert (u, v) to (x, y, z) 
// We assume u,v is in [0, 1]
//
glm::vec3 ViewPlane::toWorld(float u, float v) {
    float w = width();
    float h = height();
    return (glm::vec3((u * w) + min.x, (v * h) + min.y, position.z));
}

// Get a ray from the current camera position to the (u, v) position on
// the ViewPlane
//
Ray RenderCam::getRay(float u, float v) {
    glm::vec3 pointOnPlane = view.toWorld(u, v);
    return(Ray(position, glm::normalize(pointOnPlane - position)));
}

//--------------------------------------------------------------
void ofApp::setup() {
    theCam = &mainCam;
    previewCam.setPosition(renderCam.position);
    previewCam.lookAt(renderCam.aim);
    mainCam.setDistance(30.0);
    mainCam.lookAt(glm::vec3(0, 0, 0));
    ofSetBackgroundColor(ofColor::black);

    ofEnableDepthTest();

    //adding lights
    Light l1(glm::vec3(3, 5, 3), 10), l2(glm::vec3(-4, 3, 0), 7), l3(glm::vec3(1, 7, -4), 8);
    Light ambient(renderCam.position, 3);
    lights.push_back(ambient);
    lights.push_back(l1);
    lights.push_back(l2);
    lights.push_back(l3);

    //adding object to scene
    //scene.push_back(new Plane(glm::vec3(0, -2, 0), glm::vec3(0, 1, 0)));
    //scene.push_back(new Sphere(glm::vec3(0, 1, -3), 1.5, ofColor::greenYellow));
    //scene.push_back(new Sphere(glm::vec3(2, 2, -1), 1, ofColor::skyBlue));
    //scene.push_back(new Sphere(glm::vec3(2, 1, -6), 2, ofColor::orangeRed));
    scene.push_back(new HollowSphere(glm::vec3(0, 0, 0), ofColor::cyan));
    //scene.push_back(new Torus(glm::vec3(0, 0, -2), glm::vec2(1, 0.5), ofColor::blue));
    //scene.push_back(new Sphere(glm::vec3(0, 0, 0), 0.5, ofColor::greenYellow));

    //allocateing storage for image
    image.allocate(imageWidth, imageHeight, ofImageType::OF_IMAGE_COLOR);

    //slider setup
    gui.setup();
    gui.add(powerSlider.setup("Phong Power", 30, 2, 100));
    gui.add(ambientLightSlider.setup("Ambient light intensity", 4, 0, 8));
    gui.add(lightIntensitySlider1.setup("Light 1 intensity", 10, 1, 20));
    gui.add(lightIntensitySlider2.setup("Light 2 intensity", 7, 1, 20));
    gui.add(lightIntensitySlider3.setup("Light 3 intensity", 8, 1, 20));
}

//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {
    theCam->begin();

    //draw scene objects
    for (int i = scene.size() - 1; i >= 0; i--) {
        ofSetColor(scene[i]->diffuseColor);
        scene[i]->draw();
    }

    for (int i = 0; i < lights.size(); i++) {
        ofSetColor(lights[i].diffuseColor);
        lights[i].draw();
    }
    //   //show ray from camera
    //   for(int i = 0; i < imageWidth/10; i++)
    //      for(int j = 0; j < imageHeight/10; j++)
    //         renderCam.getRay((1.0/(imageWidth/10.0)) * i , (1.0/(imageHeight/10.0)) * j).draw(10);
    //   renderCam.getRay(0.5, 0.5).draw(20);


    ofSetColor(ofColor::white);
    renderCam.draw();
    //renderCam.view.draw();

    theCam->end();

    //draw saved image on screen
    if (bShowImage) {
        image.load("Output.png");
        image.draw(glm::vec3(0, 0, 0));
    }
    //draw slider
    ofDisableDepthTest();
    gui.draw();
    ofEnableDepthTest();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    switch (key) {
    case OF_KEY_F1:
        theCam = &mainCam;
        break;
    case OF_KEY_F2:
        theCam = &previewCam;
        break;
    case OF_KEY_F3:
        rayTrace();
        break;
    case OF_KEY_F4:
        bShowImage = !bShowImage;
        break;
    case 'm':
        rayMarchLoop();
        break;
    default:
        break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

void ofApp::progressBar(float progress, int& prevPos) {
    int barWidth = 50;
    int pos = barWidth * progress;

    if (pos != prevPos) {
        cout << "[";
        for (int i = 0; i < barWidth; i++) {
            if (i < pos)
                cout << "=";
            else if (i == pos)
                cout << ">";
            else
                cout << " ";
        }
        cout << "]" << int(progress * 100) << "%\r";
        cout.flush();
    }

    prevPos = pos;
}

//--------------------------------------------------------------
void ofApp::rayTrace() {
    //update light intensity value from slider
    lights[0].intensity = ambientLightSlider;
    lights[1].intensity = lightIntensitySlider1;
    lights[2].intensity = lightIntensitySlider2;
    lights[3].intensity = lightIntensitySlider3;

    float viewWidth = renderCam.view.width();
    float viewHeight = renderCam.view.height();
    float widthIncrament = 1.0 / imageWidth;
    float heightIncrament = 1.0 / imageHeight;
    bool hit = false;
    int closestObject;
    glm::vec3 closestIntersectPt, tempIntersectPt;
    glm::vec3 closestIntersectNorm, tempIntersectNorm;

    Ray renderRay(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0)); //placeholder

    float progress = 0.0;
    int previousPos = -1;

    //loop through each pixel
    for (int i = 1; i <= imageWidth; i++) {
        for (int j = 1; j <= imageHeight; j++) {
            //get ray for each pixel from the camera
            renderRay = renderCam.getRay(widthIncrament * (i - 1), heightIncrament * (j - 1));
            //loop though scene object to check intersection
            for (int k = 0; k < scene.size(); k++) {
                //find closest object
                if (scene[k]->intersect(renderRay, tempIntersectPt, tempIntersectNorm) && hit == false) {
                    closestObject = k;
                    closestIntersectPt = tempIntersectPt;
                    closestIntersectNorm = tempIntersectNorm;
                    hit = true;
                }
                else if (scene[k]->intersect(renderRay, tempIntersectPt, tempIntersectNorm) && hit == true) {
                    if (glm::length(renderCam.position - tempIntersectPt) <= glm::length(renderCam.position - closestIntersectPt)) {
                        closestObject = k;
                        closestIntersectPt = tempIntersectPt;
                        closestIntersectNorm = tempIntersectNorm;
                    }
                }
            }

            //after figured out closest object, get the color of the object where ray is hit
            if (hit) {
                ofColor diffuseCol = scene[closestObject]->diffuseColor;
                ofColor spectralCol = scene[closestObject]->specularColor;
                ofColor lShading = lambert(closestIntersectPt, closestIntersectNorm, diffuseCol);
                ofColor pShading = phong(closestIntersectPt, closestIntersectNorm, diffuseCol, spectralCol, powerSlider);

                image.setColor(i, imageHeight - j, pShading);
                hit = false;
            }
            else
                //backgroun color
                image.setColor(i, imageHeight - j, ofColor::black);
        }

        progress += 1.0 / imageWidth;
        if ((i + 1) == imageWidth)
            progress = 1;
        progressBar(progress, previousPos);
    }

    //save image
    image.save("Output.png");
}

//--------------------------------------------------------------
bool ofApp::rayMarching(Ray r, glm::vec3& p) {
    const int MAX_RAY_STEPS = 200;
    const float DIST_THRESHOLD = 0.001;
    const float MAX_DISTANCE = 10;
    bool hit = false;
    int objIndex;
    p = r.p;
    for (int i = 0; i < MAX_RAY_STEPS; i++) {
        float dist = sceneSDF(p, objIndex);
        if (dist < DIST_THRESHOLD) {
            hit = true;
            break;
        }
        else if (dist > MAX_DISTANCE) {
            break;
        }
        else
            p = p + (r.d * dist);
    }
    return hit;
}

//--------------------------------------------------------------
void ofApp::rayMarchLoop() {
    Ray renderRay(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0)); //placeholder
    glm::vec3 p(0,0,0);
    bool hit;
    float widthIncrament = 1.0 / imageWidth;
    float heightIncrament = 1.0 / imageHeight;

    //loop through each pixel
    for (int i = 1; i <= imageWidth; i++) {
        for (int j = 1; j <= imageHeight; j++) {
            renderRay = renderCam.getRay(widthIncrament * (i - 1), heightIncrament * (j - 1));
            hit = rayMarching(renderRay, p);
            if (hit) {
                int objIndex;
                sceneSDF(p, objIndex);
                ofColor diffuseCol = scene[objIndex]->diffuseColor;
                ofColor spectralCol = scene[objIndex]->specularColor;
                ofColor pShading = phong(p, getNormalRM(p), diffuseCol, spectralCol, powerSlider);
                image.setColor(i, imageHeight - j, pShading);
            }
            else
                image.setColor(i, imageHeight - j, ofColor::black);
        }
        cout << '.';
    }
    cout << "\n done";
    image.save("Output.png");
}

//--------------------------------------------------------------
glm::vec3 ofApp::getNormalRM(const glm::vec3& p) {
    float eps = .01;
    int objIndex;
    float dp = sceneSDF(p, objIndex);
    glm::vec3 n(dp - sceneSDF(glm::vec3(p.x - eps, p.y, p.z), objIndex),
        dp - sceneSDF(glm::vec3(p.x, p.y - eps, p.z), objIndex),
        dp - sceneSDF(glm::vec3(p.x, p.y, p.z - eps), objIndex));
    return glm::normalize(n);
}

float ofApp::opRep(const glm::vec3& p, SceneObject* obj) {
    glm::vec3 c(3);
    glm::vec3 q = glm::mod(p + 0.5 * c, c) - 0.5 * c;
    return obj->sdf(q);
}

//--------------------------------------------------------------
float ofApp::sceneSDF(const glm::vec3 p, int& objIndex) {
    float closestDist = INFINITY;
    float tempDist;
    for (int i = 0; i < scene.size(); i++) {
        tempDist = opRep(p,scene[i]);
        if (tempDist < closestDist) {
            closestDist = tempDist;
            objIndex = i;
        }
    }
    //cout << closestDist << endl;
    return closestDist;
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}

ofColor ofApp::lambert(const glm::vec3& p, const glm::vec3& n, const ofColor diffuse) {
    float lambertVal = 0;
    float diffuseCoefficient = 0.35;
    float r;
    float cosAngle;

    for (int i = 0; i < lights.size(); i++) {
        glm::vec3 l = lights[i].position;
        r = glm::length(l - p);
        cosAngle = glm::dot(n, l);
        //   cout << "position: " << p << "  normal: " << n << "  color: " << retColor << endl;
        if (0 > cosAngle)
            cosAngle = 0;

        lambertVal += diffuseCoefficient * ((lights[i].intensity / (r * r)) * cosAngle);
    }
    //getting the correct brigtness level for the diffuse color by multiplying it.
    return diffuse * lambertVal;
}

ofColor ofApp::phong(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse,
    const ofColor specular, float power) {
    //color in black, using addative process to build the image color.
    ofColor retColor = 0;
    float r, tempVal, specCoeff = 1, diffCoeff = 0.35, ambiCoeff = 1;
    //   cout << "before: " << p << " n: " << norm << "  after: " << p + (0.001 * norm) << endl;
    for (int i = 1; i < lights.size(); i++) {
        //lifted point away from surface for a small amount
        Ray pointToLight(p + (0.1 * norm), glm::normalize(lights[i].position - p));

        //if (!inShadow(pointToLight)) {
            //         retColor += lambert(p, norm, diffuse);
            r = glm::length(lights[i].position - p);
            //cosAngle part of lambart
            tempVal = glm::dot(norm, lights[i].position);
            //max between dot value and 0
            if (0 > tempVal)
                tempVal = 0;
            //lambart shading
            retColor += diffuse * (diffCoeff * ((lights[i].intensity / (r * r)) * tempVal));

            //max between 0 and dot value part of phong
            tempVal = glm::dot(norm, glm::normalize((lights[i].position - p) + (renderCam.position - p)));
            if (0 > tempVal)
                tempVal = 0;
            //         cout << tempVal << endl;
                     //phong shading
            retColor += specular * (specCoeff * (lights[i].intensity / (r * r)) * glm::pow(tempVal, power));
            //ambient lighting
            retColor += ambiCoeff * lights[0].intensity;
        //}
    }

    return retColor;
}

bool ofApp::inShadow(Ray r) {
    glm::vec3 p, n;
    for (int i = 0; i < scene.size(); i++) {
        if (rayMarching(r, p))
            return true;
    }
    return false;
}
