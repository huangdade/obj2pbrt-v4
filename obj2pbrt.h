#ifndef OBJ2PBRT_H
#define OBJ2PBRT_H

#include <vector>
#include <string>
#include <osg/Vec3>

int obj2pbrt(std::vector<std::string> args,
             osg::Vec3 eye, osg::Vec3 lookAtTarget, osg::Vec3 upDir,
             double fov,
             double scale, osg::Vec3 center);

#endif // OBJ2PBRT_H
