#include <chrono>
#include <cmath>
#include <cstring>
#include <ctime>
#include <iostream>
#include <thread>

#include "utils.h"

constexpr float min_depth = 0;
constexpr float max_depth = 100;
constexpr float width = 120;
constexpr float height = 120;

// core
inline float sdTotalScene(vec3 p) {
  vec3 transf_p = applyTransf(rotateX(-PI / 6), p);
  transf_p = applyTransf(rotateY(-PI / 2.5), transf_p);
  // return sdTorus(transf_p, 1, 0.5);
  return sdDiff(sdUnion(sdSphere(transf_p, 0.5),
                        sdDiff(sdBox(transf_p, vec3(1.2, 1.2, 1.2)),
                               sdSphere(transf_p, 1.3))),
                sdTorus(transf_p, 1.1, 0.7));
}

inline float rayMarch(vec3 camera, vec3 cam_dir) {
  float d_traveled = min_depth; // basically at the start of the screen, ie. 0
  int steps = 200;
  while (steps > 0) {
    vec3 current_pos = add(camera, scaleReal(cam_dir, d_traveled));
    float d = sdTotalScene(current_pos);
    d_traveled += d;
    if (d < EPSILON)
      break;
    if (d_traveled > max_depth)
      break;
    steps--;
  }
  return d_traveled;
}

inline void computeScreenBuffer(t_screen &screen) {
  // motivation, the bigger the screensize, the more the steps
  // we can also assign an arbitrary step
  const float dp = 1 / std::max(height, width);

  // normalized coordinates centered at (0., 0.)
  const float sy = -0.5, ey = 0.5;
  const float sx = -0.5, ex = 0.5;

  // iterate through the texture coordinate
  for (float y = sy; y < ey; y += dp) {
    for (float x = sx; x < ex; x += dp) {
      // we define a direction for each pixel
      const vec3 camera{0., 0., 5.}; // the camera must be above
      const vec3 cam_dir = normalize({x, y, -1.});

      float d_traveled = rayMarch(camera, cam_dir);
      float diffuse = 0.1; // background light
      if (d_traveled <= max_depth) {
        const vec3 contact_point = add(camera, scaleReal(cam_dir, d_traveled));
        const vec3 contact_normal = sceneNormalAt(contact_point, &sdTotalScene);
        const vec3 light_pos{3., 1., 2.};
        // light_pos = applyTransf(rotateY(-gtime), light_pos);
        // dir of the ray that hits the sphere
        vec3 light_dir = sub(light_pos, contact_point);
        light_dir = normalize(light_dir);
        diffuse = dot(light_dir, contact_normal);
      }

      const char pixel =
          screen.computeColorGivenDiffuseLight(diffuse, COLOR_STRONG);

      // texture coords ---> screen coords
      // -0.5	0.5	 ---> -width/2 width/2
      // -0.5	0.5	 ---> -height/2 height/2
      const int screen_x = (int)((x + 0.5) * width),
                screen_y = (int)((y + 0.5) * height);
      screen.put(screen_y, screen_x, pixel);
    }
  }
}

int main() {
  t_screen screen(width, height);
  computeScreenBuffer(screen);
  screen.show();
  return 0;
}