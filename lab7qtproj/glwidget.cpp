#include "glwidget.h"

QColor qcolorFromFloat(float r, float g, float b)
{
    QColor temp;
    temp.setRedF(r);
    temp.setGreenF(g);
    temp.setBlueF(b);
    return temp;
}

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent)
{

}

GLWidget::~GLWidget()
{    

}

void GLWidget::clear()
{
     updateGL();
}

void GLWidget::initializeGL()
{
    // Default scene values
    cameraPos = -100;
    maxRayRecursion = 5;
    tileSize = 1000;

    // Defaults to "Obsidian" and White from: http://globe3d.sourceforge.net/g3d_html/gl-materials__ads.htm
    floorMainMaterial = Material(qcolorFromFloat(0.05375, 0.05, 0.06625), qcolorFromFloat(0.18275, 0.17, 0.22525), qcolorFromFloat(0.332741, 0.328634, 0.346435), 38.4);
    floorSecondaryMaterial = Material(qcolorFromFloat(0.0, 0.0, 0.0), qcolorFromFloat(0.992157, 0.992157, 0.992157), qcolorFromFloat(0.0225, 0.0225, 0.0225), 12.8);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    displayImage();
}

/* 2D */
void GLWidget::resizeGL( int w, int h )
{
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho(0.0,GLdouble(w),0,GLdouble(h),-10.0,10.0);
    glFlush();
    glMatrixMode(GL_MODELVIEW);
    glViewport( 0, 0, (GLint)w, (GLint)h );
    renderWidth = w;
    renderHeight = h;
}

void GLWidget::openImage(QString fileBuf)
{     
    QImage myImage;
    myImage.load(fileBuf);
    prepareImageDisplay(&myImage);
}

/**
 * @brief GLWidget::openScene
 * @param fileBuf name of the file to open
 *
 * This function attempts to open a scene file and, if successful,
 * parses it and populates the various object lists with the information.
 *
 */
void GLWidget::openScene(QString fileBuf)
{
    QFile sceneFile(fileBuf);
    if (sceneFile.open(QFile::ReadOnly)) {
        spheres.clear();
        pointLights.clear();
        QTextStream sceneDataStream(&sceneFile);
        QString line = sceneDataStream.readLine();
        QStringList parameterValue;
        while (!line.isNull()) {
            if (line[0] != '#') { // Ignores commented lines.
                QStringList ignoreInlineComments = line.split("#");
                parameterValue = ignoreInlineComments[0].split(":");
                if (parameterValue[0] == "sphere") {
                    QStringList sphereProperties = parameterValue[1].split(",");
                    if (sphereProperties.size() == 4) {
                        spheres.append(Sphere(QVector3D(sphereProperties[1].toFloat(), sphereProperties[2].toFloat(), sphereProperties[3].toFloat()), sphereProperties[0].toFloat()));
                    } else if (sphereProperties.size() == 5) {
                        QString materialName = sphereProperties[0];
                        QVector3D spherePosition(sphereProperties[2].toFloat(), sphereProperties[3].toFloat(), sphereProperties[4].toFloat());
                        float sphereRadius = sphereProperties[1].toFloat();
                        spheres.append(Sphere(spherePosition, sphereRadius, materials[materialName]));
                    }
                } else if (parameterValue[0] == "point-light") {
                    QStringList lightProperties = parameterValue[1].split(",");
                    if (lightProperties.size() == 3) {
                        pointLights.append(PointLight(QVector3D(lightProperties[1].toFloat(), lightProperties[2].toFloat(), lightProperties[3].toFloat()),lightProperties[0].toFloat()));
                    } else if (lightProperties.size() == 6) {
                        QVector3D position(lightProperties[3].toFloat(), lightProperties[4].toFloat(), lightProperties[5].toFloat());
                        QColor colour(qRgb(lightProperties[0].toFloat(), lightProperties[1].toFloat(), lightProperties[2].toFloat()));

                        pointLights.append(PointLight(position, colour));
                    }
                } else if (parameterValue[0] == "camera-z") {
                    bool conversionSuccessful = false;
                    float temp = parameterValue[1].toFloat(&conversionSuccessful);
                    if (conversionSuccessful)
                        cameraPos = temp;
                } else if (parameterValue[0] == "ambient-light") {
                    QStringList lightProperties = parameterValue[1].split(",");
                    if (lightProperties.size() == 1) {
                        ambientColour = QColor(lightProperties[0].toInt(), lightProperties[0].toInt(), lightProperties[0].toInt());
                    } else if (lightProperties.size() == 3) {
                        ambientColour = QColor(lightProperties[0].toInt(), lightProperties[1].toInt(), lightProperties[2].toInt());
                    }
                } else if (parameterValue[0] == "max-recursion") {
                    maxRayRecursion = parameterValue[1].toInt();
                } else if (parameterValue[0] == "tile-size") {
                    tileSize = parameterValue[1].toInt();
                } else if (parameterValue[0] == "floor-main") {
                    floorMainMaterial = materials[parameterValue[1]];
                } else if (parameterValue[0] == "floor-secondary") {
                    floorSecondaryMaterial = materials[parameterValue[1]];
                } else if (parameterValue[0] == "material") {
                    QStringList materialProperties = parameterValue[1].split(",");
                    if (materialProperties.size() == 11) {
                        QColor materialAmbient(materialProperties[1].toInt(), materialProperties[2].toInt(), materialProperties[3].toInt());
                        QColor materialDiffuse(materialProperties[4].toInt(), materialProperties[5].toInt(), materialProperties[6].toInt());
                        QColor materialSpecular(materialProperties[7].toInt(), materialProperties[8].toInt(), materialProperties[9].toInt());
                        materials[materialProperties[0]] = Material(materialAmbient, materialDiffuse, materialSpecular, materialProperties[10].toFloat());
                    }
                } else if (parameterValue[0] == "material-float") {
                    QStringList materialProperties = parameterValue[1].split(",");
                    if (materialProperties.size() == 11) {
                        QColor materialAmbient = qcolorFromFloat(materialProperties[1].toFloat(), materialProperties[2].toFloat(), materialProperties[3].toFloat());
                        QColor materialDiffuse = qcolorFromFloat(materialProperties[4].toFloat(), materialProperties[5].toFloat(), materialProperties[6].toFloat());
                        QColor materialSpecular = qcolorFromFloat(materialProperties[7].toFloat(), materialProperties[8].toFloat(), materialProperties[9].toFloat());
                        materials[materialProperties[0]] = Material(materialAmbient, materialDiffuse, materialSpecular, materialProperties[10].toFloat());
                    }
                }
            }
            line = sceneDataStream.readLine();
        }
        sceneFile.close();
    } else {
        qDebug() << "File open failed";
    }
}

void GLWidget::prepareImageDisplay(QImage* myimage)
{   
    glimage = QGLWidget::convertToGLFormat( *myimage );  // flipped 32bit RGBA stored as mi
    updateGL();
}

void GLWidget::displayImage()
{
    if (glimage.width()==0) {
        return;
    } else {
        glRasterPos2i(0,0);
        glDrawPixels(glimage.width(), glimage.height(), GL_RGBA, GL_UNSIGNED_BYTE, glimage.bits());
        glFlush();
    }
}

void GLWidget::saveImage( QString fileBuf )
{
    // there is no conversion  back toQImage
    // hence the need to keep qtimage as well as glimage
    qtimage.save ( fileBuf );   // note it is the qtimage in the right format for saving
}

/**
 * @brief GLWidget::makeImage
 *
 * This is the function that loops through each pixel to determine which colour
 * value it should display.
 */
void GLWidget::makeImage( )
{   
    QImage myimage(renderWidth, renderHeight, QImage::Format_RGB32);

    QVector3D cameraPoint(renderWidth/2, renderHeight/2, cameraPos);
    for (int i = 0; i < renderWidth; i++) {
        for (int j = 0; j < renderHeight; j++) {

            QVector3D pixelPosition(i, j, 0);
            QVector3D directionVector = (pixelPosition - cameraPoint).normalized();

            QColor pixelColor = rayColor(cameraPoint, directionVector, 0);

            myimage.setPixel(i, renderHeight - 1 -j, pixelColor.rgb());
        }
    }

    qtimage=myimage.copy(0, 0,  myimage.width(), myimage.height());

    prepareImageDisplay(&myimage);
}

/**
 * @brief intersectSphere
 * @param initialPoint
 * @param direction
 * @param intersectionPoint passed in by reference, this will be updated to the point where the ray intersects the sphere
 * @return the distance to the closest sphere intersected, or -1 if none.
 *
 * Utility function to easily test for ray-sphere intersection.
 */
int GLWidget::intersectSpheres(QVector3D initialPosition, QVector3D direction, QVector3D& intersectionPoint)
{
    float smallestT = -1;
    int closestSphereIndex = -1;
    // Loop through every object to test for collision
    for (int i = 0; i < spheres.size(); i++) {
        float sphereRadius = spheres[i].radius;
        QVector3D sphereOrigin = spheres[i].origin;
        QVector3D rayOriginMinusSphereCenter = initialPosition - sphereOrigin;
        // Ray: R = CameraPoint + t D;

        float partA = direction.lengthSquared();
        float partB = QVector3D::dotProduct(direction, rayOriginMinusSphereCenter);
        float partC = rayOriginMinusSphereCenter.lengthSquared() - (sphereRadius*sphereRadius);

        float discriminant = (partB * partB) - (partA * partC);
        if (discriminant >= 0) {
            float t = (-1*partB -  sqrt(discriminant)) / partA;
            if (t > 0) {
                if (t < smallestT || smallestT == -1) {
                    smallestT = t;
                    closestSphereIndex = i;
                }
            }
        }
    }
    if (closestSphereIndex > -1) {
        intersectionPoint = initialPosition + (smallestT * direction.normalized());
    }
    return closestSphereIndex;

}

/**
 * @brief GLWidget::rayColor
 * @param rayOrigin
 * @param rayDirection
 * @param timesCalled a variable that should be incremented each time this method is called recursively.
 * @return a QColor object representing the colour that the ray's intersections determined.
 *
 * Method to calculate what colour a shot ray would generate. Called recursively to account for reflections.
 */
QColor GLWidget::rayColor(QVector3D rayOrigin, QVector3D rayDirection, int timesCalled) {

    int closestSphereIndex = -1;
    QVector3D intersectionPoint;

    closestSphereIndex = intersectSpheres(rayOrigin, rayDirection, intersectionPoint);
    QColor pixelColour(0,0,0);

    QVector3D normalVector;
    QVector3D viewVector;

    Material hitMaterial;
    bool calculateLight = false;

    if (closestSphereIndex > -1) {
        // We have a collision, now we must determine the proper colour

        Sphere hitSphere = spheres[closestSphereIndex];

        // cameraPoint + smallestT * directionVector is the point on the closest sphere.
        normalVector = (intersectionPoint - hitSphere.origin).normalized();
        viewVector = (rayOrigin - intersectionPoint).normalized();

        hitMaterial = hitSphere.material;

        calculateLight = true;
    } else {
        // No intersection with a sphere, see if we intersect with the floor.
        // distance t = (planeNormal dot (point on plane - rayOrigin)) / (planeNormal dot rayDirection)
        normalVector = QVector3D(0,1,0);
        float t = QVector3D::dotProduct(normalVector, (QVector3D(0,0,0) - rayOrigin)) / (QVector3D::dotProduct(normalVector, rayDirection));
        if (t > 0 && t < VIEW_DISTANCE) {
            intersectionPoint = rayOrigin + t*rayDirection.normalized();
            viewVector = (rayOrigin - intersectionPoint).normalized();
            bool mainColour = true;

            if ((int)(intersectionPoint.x() / tileSize) % 2 == 0)
                mainColour = !mainColour;
            if ((int)(intersectionPoint.z() / tileSize) % 2 == 0)
                mainColour = !mainColour;

            hitMaterial = mainColour ? floorMainMaterial : floorSecondaryMaterial;

            calculateLight = true;
        }
    }

    if (calculateLight) {
        float pixelR = hitMaterial.ambient.redF()*ambientColour.redF();
        float pixelG = hitMaterial.ambient.greenF()*ambientColour.greenF();
        float pixelB = hitMaterial.ambient.blueF()*ambientColour.blueF();

        for ( int i = 0; i < pointLights.size(); i++) {
            QVector3D lightVector = (pointLights[i].position - intersectionPoint).normalized();
            QVector3D shadowCollisionPoint;
            // Test for objects in between light and intersection point
            int shadowSphereIndex = intersectSpheres(intersectionPoint + lightVector, lightVector, shadowCollisionPoint);
            if (shadowSphereIndex == -1) { // No intersection
                //LAMBERTIAN DIFFUSE SHADING:
                // L += surface Colour x lightIntensity x max(0,normal.lightVector)
                // lightVector is computed by subtracting intersection point from the light source position
                float nDotL = (float)QVector3D::dotProduct(normalVector,lightVector);
                if ( nDotL > 0) {
                    pixelR += hitMaterial.diffuse.redF()*pointLights[i].colour.redF()*nDotL;
                    pixelG += hitMaterial.diffuse.greenF()*pointLights[i].colour.greenF()*nDotL;
                    pixelB += hitMaterial.diffuse.blueF()*pointLights[i].colour.blueF()*nDotL;
                }

                //BLINN-PHONG SPECULAR SHADING:
                // L += surface specular * lightIntensity * max(0,normal.h)^p
                // h is (v + l) normalized.
                // p is a Phong exponent, defined on the sphere.
                QVector3D halfVector = (viewVector + lightVector).normalized();
                float nDotH = (float)QVector3D::dotProduct(normalVector,halfVector);
                if (nDotH > 0) {
                    nDotH = pow(nDotH, hitMaterial.phongExponent);
                    // TODO-DG: Figure out what to use for the specular colour of the object.
                    pixelR += hitMaterial.specular.redF()*pointLights[i].colour.redF()*nDotH;
                    pixelG += hitMaterial.specular.greenF()*pointLights[i].colour.greenF()*nDotH;
                    pixelB += hitMaterial.specular.blueF()*pointLights[i].colour.blueF()*nDotH;
                }
            }
            //SPECULAR REFLECTION:
            if (timesCalled < maxRayRecursion) {
                QVector3D reflection = rayDirection - 2.0f*(QVector3D::dotProduct(rayDirection, normalVector))*normalVector;
                // Add specularColor*(recursive call to raycolor) to our colour.
                QColor tempColor = rayColor(intersectionPoint + reflection.normalized(), reflection.normalized(), timesCalled++);
                pixelR += hitMaterial.specular.redF()*tempColor.redF();
                pixelG += hitMaterial.specular.greenF()*tempColor.greenF();
                pixelB += hitMaterial.specular.blueF()*tempColor.blueF();
            }
        }

        // TODO-DG: Currently this crudely adds the effects of the last light in the scene.
        // TODO-DG: Update to take an average of the entire screen
        if (pixelR > 1.0f)
            pixelR = 1.0f;
        if (pixelG > 1.0f)
            pixelG = 1.0f;
        if (pixelB > 1.0f)
            pixelB = 1.0f;

       pixelColour = QColor(pixelR*255, pixelG*255, pixelB*255);
    }

    return pixelColour;
}

