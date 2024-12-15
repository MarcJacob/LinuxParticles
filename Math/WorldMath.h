// 3 dimensional vector with an extra w component to allow for transformations.
// Can represent a point or a direction in normal world space of basis X = (1, 0, 0, 1), Y = (0, 1, 0, 1), Z = (0, 0, 1, 1)
struct WorldVector
{
  // Common values
  static const WorldVector ForwardVector;
  static const WorldVector RightVector;
  static const WorldVector UpVector;
  static const WorldVector OneVector;
  static const WorldVector ZeroVector;

  void operator =(const WorldVector& b)
  {
    x = b.x;
    y = b.y;
    z = b.z;
    w = b.w;
  }
  
  // Data
  float x = 0.f, y = 0.f, z = 0.f, w = 1.f;
};


// WorldVector Operations
template<typename Scalar>
static WorldVector operator *(const WorldVector& v, Scalar s)
{
  return {v.x * s, v.y * s, v.z * s, 1.f};
}

template<typename Scalar>
static WorldVector operator /(const WorldVector& v, Scalar s)
{
  return {v.x / s, v.y / s, v.z / s, 1.f};
}

static WorldVector operator +(const WorldVector& a, const WorldVector& b)
{
  return {a.x + b.x, a.y + b.y, a.z + b.z, 1.f};
}

static WorldVector operator-(const WorldVector& v)
{
  return {-v.x, -v.y, -v.z};
}

static WorldVector operator -(const WorldVector& a, const WorldVector& b)
{
  return {a.x - b.x, a.y - b.y, a.z - b.z, 1.f};
}

static float DotProduct(const WorldVector& a, const WorldVector& b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

static WorldVector CrossProduct(const WorldVector& a, const WorldVector& b)
{
  return
    {
      a.y * b.z - a.z * b.y,
      a.x * b.z - a.z * b.x,
      a.y * b.x - a.x * b.y,
      1.f
    };
}

static float LengthSquared(const WorldVector& v)
{
  return v.x * v.x + v.y * v.y + v.z * v.z;
}

static WorldVector NormalizeVector(const WorldVector& v)
{
  return v / sqrt(LengthSquared(v));
}

const WorldVector WorldVector::ForwardVector {1, 0, 0, 1};
const WorldVector WorldVector::RightVector {0, 1, 0, 1};
const WorldVector WorldVector::UpVector {0, 0, 1, 1};
const WorldVector WorldVector::OneVector {1, 1, 1, 1};
const WorldVector WorldVector::ZeroVector {0, 0, 0, 0};


struct Matrix4x4
{
  // Common values
  static const Matrix4x4 Identity;
  
  // Data
  union
  {
    float LinearMatrix[16];
    float Matrix[4][4];
  };

  float* operator[](int index)
  {
    return Matrix[index];
  }

  const float* operator[](int index) const
  {
    return Matrix[index];
  }
  
  void SetTranslation(float x, float y, float z)
  {
    Matrix[0][3] = x;
    Matrix[1][3] = y;
    Matrix[2][3] = z;
  }

  void SetTranslation(WorldVector vec)
  {
    SetTranslation(vec.x, vec.y, vec.z);
  }

  void AddTranslation(WorldVector vec)
  {
    Matrix[0][3] += vec.x;
    Matrix[1][3] += vec.y;
    Matrix[2][3] += vec.z;
    Matrix[3][3] += vec.w;
  }

  void SetScale(float x, float y, float z)
  {
    Matrix[0][0] = x;
    Matrix[1][1] = y;
    Matrix[2][2] = z;
  }

  WorldVector GetTranslation()
  {
    return WorldVector {Matrix[0][3], Matrix[1][3], Matrix[2][3], Matrix[3][3]};
  }
};

// Matrix4x4 Operations 

static WorldVector operator*(const Matrix4x4& matrix, const WorldVector& vec)
{
  return
    {
      vec.x * matrix[0][0] + vec.y * matrix[0][1] + vec.z * matrix[0][2] + vec.w * matrix[0][3],
      vec.x * matrix[1][0] + vec.y * matrix[1][1] + vec.z * matrix[1][2] + vec.w * matrix[1][3],
      vec.x * matrix[2][0] + vec.y * matrix[2][1] + vec.z * matrix[2][2] + vec.w * matrix[2][3],
      vec.x * matrix[3][0] + vec.y * matrix[3][1] + vec.z * matrix[3][2] + vec.w * matrix[3][3],
    };
}

const Matrix4x4 Matrix4x4::Identity {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  };
