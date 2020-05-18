  // Look At matrix creation
  // return the inverse view matrix
  //
  static inline float4x4 lookAt(float3 eye, float3 center, float3 up)
  {
    float3 x, y, z; // basis; will make a rotation matrix

    z.x = eye.x - center.x;
    z.y = eye.y - center.y;
    z.z = eye.z - center.z;
    z = normalize(z);

    y.x = up.x;
    y.y = up.y;
    y.z = up.z;

    x = cross(y, z); // X vector = Y cross Z
    y = cross(z, x); // Recompute Y = Z cross X

    // cross product gives area of parallelogram, which is < 1.0 for
    // non-perpendicular unit-length vectors; so normalize x, y here
    x = normalize(x);
    y = normalize(y);

    float4x4 M;
    M.set_col(0, float4{ x.x, y.x, z.x, 0.0f });
    M.set_col(1, float4{ x.y, y.y, z.y, 0.0f });
    M.set_col(2, float4{ x.z, y.z, z.z, 0.0f });
    M.set_col(3, float4{ -x.x * eye.x - x.y * eye.y - x.z*eye.z,
                         -y.x * eye.x - y.y * eye.y - y.z*eye.z,
                         -z.x * eye.x - z.y * eye.y - z.z*eye.z,
                         1.0f });
    return M;
  }

  static inline float4x4 perspectiveMatrix(float fovy, float aspect, float zNear, float zFar)
  {
    const float ymax = zNear * tanf(fovy * 3.14159265358979323846f / 360.0f);
    const float xmax = ymax * aspect;

    const float left = -xmax;
    const float right = +xmax;
    const float bottom = -ymax;
    const float top = +ymax;

    const float temp = 2.0f * zNear;
    const float temp2 = right - left;
    const float temp3 = top - bottom;
    const float temp4 = zFar - zNear;

    float4x4 res;
    res.set_col(0, float4{ temp / temp2, 0.0f, 0.0f, 0.0f });
    res.set_col(1, float4{ 0.0f, temp / temp3, 0.0f, 0.0f });
    res.set_col(2, float4{ (right + left) / temp2,  (top + bottom) / temp3, (-zFar - zNear) / temp4, -1.0 });
    res.set_col(3, float4{ 0.0f, 0.0f, (-temp * zFar) / temp4, 0.0f });
    return res;
  }
