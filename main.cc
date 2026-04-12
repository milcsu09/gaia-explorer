#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>


///////////////////////////////////////////////////////////////////////////////


#if 0
constexpr int WW = 800;
constexpr int WH = 600;
#else
constexpr int WW = 1920;
constexpr int WH = 1080;
#endif


constexpr bool VSYNC      = false;
constexpr bool SHOW_MOUSE = false;


///////////////////////////////////////////////////////////////////////////////


static constexpr double PI   = 3.14159265358979323846;
static constexpr double PI_2 = 1.57079632679489661923;
static constexpr double PI_4 = 0.78539816339744830962;

#define RAD(a) ((a) * (PI / 180.0))
#define DEG(a) ((a) / (PI / 180.0))


static constexpr double SOL = 299'792.0 /* km/h */;


inline double
clamp (double x, double a, double b)
{
  if (x < a)
    return a;

  if (x > b)
    return b;

  return x;
}


///////////////////////////////////////////////////////////////////////////////


using kilometer = int64_t;


// ----------------------------------------------------------------------------


class spatial_unit
{
private:
  kilometer m_value{ 0 };

public:
  static constexpr kilometer AU  =            149'597'871 /* km */;
  static constexpr kilometer LY  =      9'460'730'472'581 /* km */;
  static constexpr kilometer PC  =     30'856'775'814'914 /* km */;
  static constexpr kilometer KPC = 30'856'775'814'914'000 /* km */;

  spatial_unit () = default;

  spatial_unit (kilometer value);

  static spatial_unit
  from_km (kilometer value);

  static spatial_unit
  from_au (double value);

  static spatial_unit
  from_ly (double value);

  static spatial_unit
  from_pc (double value);

  static spatial_unit
  from_kpc (double value);

  kilometer
  to_km () const;

  double
  to_au () const;

  double
  to_ly () const;

  double
  to_pc () const;

  double
  to_kpc () const;

  bool
  operator== (const spatial_unit &other) const;

  bool
  operator!= (const spatial_unit &other) const;

  bool
  operator< (const spatial_unit &other) const;

  bool
  operator> (const spatial_unit &other) const;

  bool
  operator<= (const spatial_unit &other) const;

  bool
  operator>= (const spatial_unit &other) const;

  spatial_unit
  operator+ (const spatial_unit &other) const;

  spatial_unit
  operator- () const;

  spatial_unit
  operator- (const spatial_unit &other) const;

  spatial_unit
  operator* (double scalar) const;

  spatial_unit
  operator/ (double scalar) const;

  spatial_unit &
  operator+= (const spatial_unit &other);

  spatial_unit &
  operator-= (const spatial_unit &other);

  spatial_unit &
  operator*= (double scalar);

  spatial_unit &
  operator/= (double scalar);

  friend spatial_unit
  operator* (double scalar, const spatial_unit &unit);
};


// ----------------------------------------------------------------------------


spatial_unit::spatial_unit (kilometer value)
  : m_value (value)
{
}


spatial_unit
spatial_unit::from_km (kilometer value)
{
  return spatial_unit (value);
}


spatial_unit
spatial_unit::from_au (double value)
{
  return spatial_unit (std::llround (value * AU));
}


spatial_unit
spatial_unit::from_ly (double value)
{
  return spatial_unit (std::llround (value * LY));
}


spatial_unit
spatial_unit::from_pc (double value)
{
  return spatial_unit (std::llround (value * PC));
}


spatial_unit
spatial_unit::from_kpc (double value)
{
  return spatial_unit (std::llround (value * KPC));
}


kilometer
spatial_unit::to_km () const
{
  return m_value;
}


double
spatial_unit::to_au () const
{
  return static_cast<double> (m_value) / AU;
}


double
spatial_unit::to_ly () const
{
  return static_cast<double> (m_value) / LY;
}


double
spatial_unit::to_pc () const
{
  return static_cast<double> (m_value) / PC;
}


double
spatial_unit::to_kpc () const
{
  return static_cast<double> (m_value) / KPC;
}


bool
spatial_unit::operator== (const spatial_unit &other) const
{
  return m_value == other.m_value;
}


bool
spatial_unit::operator!= (const spatial_unit &other) const
{
  return m_value != other.m_value;
}


bool
spatial_unit::operator< (const spatial_unit &other) const
{
  return m_value < other.m_value;
}


bool
spatial_unit::operator> (const spatial_unit &other) const
{
  return m_value > other.m_value;
}


bool
spatial_unit::operator<= (const spatial_unit &other) const
{
  return m_value <= other.m_value;
}


bool
spatial_unit::operator>= (const spatial_unit &other) const
{
  return m_value >= other.m_value;
}


spatial_unit
spatial_unit::operator+ (const spatial_unit &other) const
{
  return spatial_unit (m_value + other.m_value);
}


spatial_unit
spatial_unit::operator- () const
{
  return spatial_unit (0 - m_value);
}


spatial_unit
spatial_unit::operator- (const spatial_unit &other) const
{
  return spatial_unit (m_value - other.m_value);
}


spatial_unit
spatial_unit::operator* (double scalar) const
{
  return spatial_unit (std::llround (m_value * scalar));
}


spatial_unit
spatial_unit::operator/ (double scalar) const
{
  return spatial_unit (std::llround (m_value / scalar));
}


spatial_unit &
spatial_unit::operator+= (const spatial_unit &other)
{
  m_value = m_value + other.m_value;
  return *this;
}


spatial_unit &
spatial_unit::operator-= (const spatial_unit &other)
{
  m_value = m_value - other.m_value;
  return *this;
}


spatial_unit &
spatial_unit::operator*= (double scalar)
{
  m_value = std::llround (m_value * scalar);
  return *this;
}


spatial_unit &
spatial_unit::operator/= (double scalar)
{
  m_value = std::llround (m_value / scalar);
  return *this;
}


spatial_unit
operator* (double scalar, const spatial_unit &unit)
{
  return unit * scalar;
}


///////////////////////////////////////////////////////////////////////////////


template <typename T> struct vector3
{
  static const vector3<T> ZERO;

  T x{ 0 }, y{ 0 }, z{ 0 };

  vector3 () = default;

  vector3 (T v);

  vector3 (T _x, T _y, T _z);

  vector3 (const vector3<T> &other);

  vector3 (vector3<T> &&other) noexcept;

  vector3<T> &
  operator= (const vector3<T> &other);

  vector3<T> &
  operator= (vector3<T> &&other) noexcept;

  std::common_type_t<T, double>
  distance (const vector3<T> &other) const;

  vector3<T>
  operator+ (const vector3<T> &other) const;

  vector3<T>
  operator- (const vector3<T> &other) const;

  vector3<T>
  operator* (double scalar) const;

  vector3<T>
  operator/ (double scalar) const;

  vector3<T> &
  operator+= (const vector3<T> &other);

  vector3<T> &
  operator-= (const vector3<T> &other);

  vector3<T> &
  operator*= (double scalar);

  vector3<T> &
  operator/= (double scalar);

  template <typename U>
  friend vector3<U>
  operator* (double scalar, const vector3<U> &v);
};


// ----------------------------------------------------------------------------


typedef vector3<int64_t> vector3i;

typedef vector3<double> vector3f;

typedef vector3<spatial_unit> vector3su;


// ----------------------------------------------------------------------------


template <typename T> const vector3<T> vector3<T>::ZERO{ 0, 0, 0 };


template <typename T>
vector3<T>::vector3 (T v)
  : x (v),
    y (v),
    z (v)
{
}


template <typename T>
vector3<T>::vector3 (T _x, T _y, T _z)
  : x (_x),
    y (_y),
    z (_z)
{
}


template <typename T>
vector3<T>::vector3 (const vector3<T> &other)
  : x (other.x),
    y (other.y),
    z (other.z)
{
}


template <typename T>
vector3<T>::vector3 (vector3<T> &&other) noexcept
  : x (std::move (other.x)),
    y (std::move (other.y)),
    z (std::move (other.z))
{
}


template <typename T>
vector3<T> &
vector3<T>::operator= (const vector3<T> &other)
{
  x = other.x;
  y = other.y;
  z = other.z;
  return *this;
}


template <typename T>
vector3<T> &
vector3<T>::operator= (vector3<T> &&other) noexcept
{
  x = std::move (other.x);
  y = std::move (other.y);
  z = std::move (other.z);
  return *this;
}


template <typename T>
std::common_type_t<T, double>
vector3<T>::distance (const vector3<T> &other) const
{
  const auto dx = x - other.x;
  const auto dy = y - other.y;
  const auto dz = z - other.z;
  return std::sqrt (dx * dx + dy * dy + dz * dz);
}


template <typename T>
vector3<T>
vector3<T>::operator+ (const vector3<T> &other) const
{
  return vector3 (x + other.x, y + other.y, z + other.z);
}


template <typename T>
vector3<T>
vector3<T>::operator- (const vector3<T> &other) const
{
  return vector3 (x - other.x, y - other.y, z - other.z);
}


template <typename T>
vector3<T>
vector3<T>::operator* (double scalar) const
{
  return vector3 (x * scalar, y * scalar, z * scalar);
}


template <typename T>
vector3<T>
vector3<T>::operator/ (double scalar) const
{
  return vector3 (x / scalar, y / scalar, z / scalar);
}


template <typename T>
vector3<T> &
vector3<T>::operator+= (const vector3<T> &other)
{
  x += other.x;
  y += other.y;
  z += other.z;
  return *this;
}


template <typename T>
vector3<T> &
vector3<T>::operator-= (const vector3<T> &other)
{
  x -= other.x;
  y -= other.y;
  z -= other.z;
  return *this;
}


template <typename T>
vector3<T> &
vector3<T>::operator*= (double scalar)
{
  x *= scalar;
  y *= scalar;
  z *= scalar;
  return *this;
}


template <typename T>
vector3<T> &
vector3<T>::operator/= (double scalar)
{
  x /= scalar;
  y /= scalar;
  z /= scalar;
  return *this;
}


template <typename T>
vector3<T>
operator* (double scalar, const vector3<T> &v)
{
  return v * scalar;
}


///////////////////////////////////////////////////////////////////////////////


static std::string
fmt (double v)
{
  char buffer[256];

  std::snprintf (buffer, sizeof buffer, "%.1f", v);

  return buffer;
};


std::string
to_human (double x)
{
  if (x == 0)
    return fmt (x);

  const auto ax = std::abs (x);

  if (ax >= 1e12)
    return fmt (x * 1e-12) + "T";

  if (ax >= 1e9)
    return fmt (x * 1e-9) + "G";

  if (ax >= 1e6)
    return fmt (x * 1e-6) + "M";

  if (ax >= 1e3)
    return fmt (x * 1e-3) + "k";

  if (ax >= 1)
    return fmt (x);

  if (ax >= 1e-3)
    return fmt (x * 1e3) + "m";

  if (ax >= 1e-6)
    return fmt (x * 1e6) + "µ";

  if (ax >= 1e-9)
    return fmt (x * 1e9) + "n";

  return fmt (x * 1e12) + "p";
}


std::string
to_human (spatial_unit x)
{
  const double km = static_cast<double> (x.to_km ());
  const double ax = std::abs (km);

  if (ax >= 0.1 * spatial_unit::KPC)
    return fmt (km / spatial_unit::KPC) + " kpc";

  if (ax >= 0.1 * spatial_unit::LY)
    return fmt (km / spatial_unit::LY) + " ly";

  if (ax >= 0.1 * spatial_unit::AU)
    return fmt (km / spatial_unit::AU) + " AU";

  return fmt (km) + " km";
}


// sf::String
// to_sf_string (const std::string &s)
// {
//   return sf::String::fromUtf8 (s.begin (), s.end ());
// }


///////////////////////////////////////////////////////////////////////////////


sf::Color
temperature_to_color (double temp)
{
  double T = temp / 100.0;

  double r, g, b;

  // Red
  if (T <= 66)
    r = 255;
  else
    r = 329.698727446 * pow (T - 60, -0.1332047592);

  // Green
  if (T <= 66)
    g = 99.4708025861 * log (T) - 161.1195681661;
  else
    g = 288.1221695283 * pow (T - 60, -0.0755148492);

  // Blue
  if (T >= 66)
    b = 255;
  else if (T <= 19)
    b = 0;
  else
    b = 138.5177312231 * log (T - 10) - 305.0447927307;

  return {
    static_cast<sf::Uint8> (clamp (r, 0.0, 255.0)),
    static_cast<sf::Uint8> (clamp (g, 0.0, 255.0)),
    static_cast<sf::Uint8> (clamp (b, 0.0, 255.0)),
  };
}


///////////////////////////////////////////////////////////////////////////////



struct Gaia_Star
{
  int64_t source_id;
  double  ra           /* degree */;
  double  dec          /* degree */;
  double  parallax     /* milliarcsecond */;
  double  lum_flame    /* star luminosity */;
  double  teff_gspphot /* temperature */;

  Gaia_Star () = default;

  Gaia_Star (std::string line);
};


// ----------------------------------------------------------------------------


Gaia_Star::Gaia_Star (std::string line)
{
  std::stringstream ss (line);
  std::string token;

  std::getline (ss, token, ',');
  source_id = std::stoll (token);

  std::getline (ss, token, ',');
  ra = std::stod (token);

  std::getline (ss, token, ',');
  dec = std::stod (token);

  std::getline (ss, token, ',');
  parallax = std::stod (token);

  std::getline (ss, token, ',');
  lum_flame = std::stod (token);

  std::getline (ss, token, ',');
  teff_gspphot = std::stod (token);
}


///////////////////////////////////////////////////////////////////////////////


struct Star
{
  // vector3su position;

  double x /* pc */;
  double y /* pc */;
  double z /* pc */;

  double lum;
  double teff;

  sf::Color tint;

  Star () = default;

  Star (Gaia_Star gaia_star);
};


// ----------------------------------------------------------------------------


Star::Star (Gaia_Star gaia_star)
{
  if (gaia_star.parallax <= 0.0)
    return;

  double r_rad = RAD (gaia_star.ra);
  double d_rad = RAD (gaia_star.dec);

  double distance_pc = 1000.0 / gaia_star.parallax;

  x = distance_pc * std::cos (d_rad) * std::cos (r_rad);
  y = distance_pc * std::cos (d_rad) * std::sin (r_rad);
  z = distance_pc * std::sin (d_rad);

  lum  = gaia_star.lum_flame;
  teff = gaia_star.teff_gspphot;

  tint = temperature_to_color (teff);
}


///////////////////////////////////////////////////////////////////////////////


bool
import_gaia (std::vector<Star> &stars, const std::string &path)
{
  std::ifstream file (path);

  if (!file.is_open ())
    return false;

  std::string line;

  std::getline (file, line); // Header

  while (std::getline (file, line))
    stars.emplace_back (/* Star{} */ Gaia_Star (line));

  if (file.bad ())
    return false;

  if (!file.eof ())
    return false;

  return true;
}


///////////////////////////////////////////////////////////////////////////////


struct Camera
{
  vector3su position;
  double ra  /* degree */;
  double dec /* degree */;
  double fov /* degree */;

  double exposure{ 1.0 / 16.0 };
};


///////////////////////////////////////////////////////////////////////////////


struct Cache
{
  double r0x, r0y;
  double r1x, r1y, r1z;
  double r2x, r2y, r2z;

  double scale_x, offset_x;
  double scale_y, offset_y;

  double aspect;
  double inv_half_h;

  Cache (const Camera &camera);
};


Cache::Cache (const Camera &camera)
{
  const double r_rad = RAD (camera.ra);
  const double d_rad = RAD (camera.dec);

  const double r_sin = std::sin (r_rad);
  const double r_cos = std::cos (r_rad);

  const double d_sin = std::sin (d_rad);
  const double d_cos = std::cos (d_rad);

  r0x =  r_sin;
  r0y = -r_cos;

  r1x = -d_sin * r_cos;
  r1y = -d_sin * r_sin;
  r1z =  d_cos;

  r2x =  d_cos * r_cos;
  r2y =  d_cos * r_sin;
  r2z =  d_sin;

  const double half_h = std::tan (RAD (camera.fov * 0.5));

  inv_half_h = 1.0f / half_h;

  aspect = static_cast<double> (WW) / static_cast<double> (WH);

  scale_x  =  0.5 * WW / (half_h * aspect);
  offset_x =  0.5 * WW;

  scale_y  = -0.5 * WH / half_h;
  offset_y =  0.5 * WH;
}


inline bool
project (double dx, double dy, double dz, const Cache &cache, double &px, double &py)
{
  const double cz = dx * cache.r2x + dy * cache.r2y + dz * cache.r2z;

  if (cz <= 0.0)
    return false;

  const double cx = dx * cache.r0x + dy * cache.r0y;
  const double cy = dx * cache.r1x + dy * cache.r1y + dz * cache.r1z;

  const double inv_cz = 1.0 / cz;

  const double x = cx * inv_cz;
  const double y = cy * inv_cz;

  px = x * cache.inv_half_h / cache.aspect;
  py = y * cache.inv_half_h;

  return true;
}


///////////////////////////////////////////////////////////////////////////////


struct Mouse
{
  static constexpr int X_MIN = 0;
  static constexpr int Y_MIN = 0;
  static constexpr int X_MAX = WW - 1;
  static constexpr int Y_MAX = WH - 1;

  sf::Vector2i position{ 0, 0 };
  sf::Vector2i previous{ 0, 0 };

  sf::Vector2i delta{ 0, 0 };

  void
  update (const sf::RenderWindow &window)
  {
    position = sf::Mouse::getPosition (window);

    delta = position - previous;

    // NOTE: Wrapping logic is necessary for calculating precise mouse delta.
    if (position.x >= X_MAX)
      sf::Mouse::setPosition ({ X_MIN + 1, position.y }, window);

    if (position.x <= X_MIN)
      sf::Mouse::setPosition ({ X_MAX - 1, position.y }, window);

    if (position.y >= Y_MAX)
      sf::Mouse::setPosition ({ position.x, Y_MIN + 1 }, window);

    if (position.y <= Y_MIN)
      sf::Mouse::setPosition ({ position.x, Y_MAX - 1 }, window);

    previous = sf::Mouse::getPosition (window);
  }
};


///////////////////////////////////////////////////////////////////////////////


constexpr const char *VS_BASIC_COLOR = R"(
#version 330 core

layout (location = 0) in vec2 a_position;
layout (location = 1) in vec4 a_color;

out vec4 v_color;

void main ()
{
  gl_Position = vec4 (a_position, 0.0, 1.0);

  v_color = a_color;
}
)";


constexpr const char *FS_BASIC_COLOR = R"(
#version 330 core

in vec4 v_color;

out vec4 f_color;

void main ()
{
  f_color = v_color;
}
)";


// ----------------------------------------------------------------------------


constexpr const char *VS_BASIC_TEXTURE = R"(
#version 330 core

layout (location = 0) in vec2 a_position;
layout (location = 1) in vec2 a_uv;

out vec2 v_uv;

void main ()
{
  gl_Position = vec4 (a_position, 0.0, 1.0);

  v_uv = a_uv;
}
)";


constexpr const char *FS_BASIC_TEXTURE = R"(
#version 330 core

uniform sampler2D u_texture;

in vec2 v_uv;

out vec4 f_color;

void main ()
{
  f_color = texture2D (u_texture, v_uv);
}
)";



// ----------------------------------------------------------------------------


constexpr const char *VS_BLUR = R"(
#version 330 core

layout (location = 0) in vec2 a_position;

void main ()
{
  gl_Position = vec4 (a_position, 0.0, 1.0);
}
)";


// No fancy algorithm / math behind this shader; just changing numbers until they looked good.
constexpr const char *FS_BLUR = R"(
#version 120

uniform sampler2D u_texture;
uniform vec2 u_direction;
uniform vec2 u_texel;

void main()
{
  vec2 uv   = gl_FragCoord.xy * u_texel;
  vec2 step = u_direction * u_texel;

  vec4 result = vec4 (0.0);

  // Weights generated from `(1 / 2.0 ** abs (x)) / sum (weights)`

  result += texture2D (u_texture, uv - 10 * step) * 0.0003257328990228013;
  result += texture2D (u_texture, uv - 9 * step) * 0.0006514657980456026;
  result += texture2D (u_texture, uv - 8 * step) * 0.0013029315960912053;
  result += texture2D (u_texture, uv - 7 * step) * 0.0026058631921824105;
  result += texture2D (u_texture, uv - 6 * step) * 0.005211726384364821;
  result += texture2D (u_texture, uv - 5 * step) * 0.010423452768729642;
  result += texture2D (u_texture, uv - 4 * step) * 0.020846905537459284;
  result += texture2D (u_texture, uv - 3 * step) * 0.04169381107491857;
  result += texture2D (u_texture, uv - 2 * step) * 0.08338762214983714;
  result += texture2D (u_texture, uv - 1 * step) * 0.16677524429967427;
  result += texture2D (u_texture, uv)            * 0.33355048859934855;
  result += texture2D (u_texture, uv + 1 * step) * 0.16677524429967427;
  result += texture2D (u_texture, uv + 2 * step) * 0.08338762214983714;
  result += texture2D (u_texture, uv + 3 * step) * 0.04169381107491857;
  result += texture2D (u_texture, uv + 4 * step) * 0.020846905537459284;
  result += texture2D (u_texture, uv + 5 * step) * 0.010423452768729642;
  result += texture2D (u_texture, uv + 6 * step) * 0.005211726384364821;
  result += texture2D (u_texture, uv + 7 * step) * 0.0026058631921824105;
  result += texture2D (u_texture, uv + 8 * step) * 0.0013029315960912053;
  result += texture2D (u_texture, uv + 9 * step) * 0.0006514657980456026;
  result += texture2D (u_texture, uv + 10 * step) * 0.0003257328990228013;

  gl_FragColor = result * 4;
}
)";


///////////////////////////////////////////////////////////////////////////////


struct Framebuffer
{
  enum struct Format : GLenum
  {
    RGBA16F = GL_RGBA16F,
    RGBA32F = GL_RGBA32F,
  };

  GLuint fbo{ 0 };
  GLuint texture{ 0 };
  GLuint depth{ 0 };

  uint32_t w{ 0 };
  uint32_t h{ 0 };

  Format format{ Format::RGBA16F };

  Framebuffer (uint32_t _w, uint32_t _h, Format _format = Format::RGBA16F);

  ~Framebuffer ();

  void
  bind () const;

  void
  unbind () const;

  void
  unbind (uint32_t _w, uint32_t _h) const;
};


// ----------------------------------------------------------------------------


Framebuffer::Framebuffer (uint32_t _w, uint32_t _h, Format _format)
  : w (_w),
    h (_h),
    format (_format)
{
  glGenFramebuffers (1, &fbo);
  glBindFramebuffer (GL_FRAMEBUFFER, fbo);

  glGenTextures (1, &texture);
  glBindTexture (GL_TEXTURE_2D, texture);

  glTexImage2D (
    GL_TEXTURE_2D,
    0,
    static_cast<GLenum> (format),
    static_cast<GLsizei> (_w),
    static_cast<GLsizei> (_h),
    0,
    GL_RGBA,
    GL_FLOAT,
    nullptr
  );
 
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glFramebufferTexture2D (
    GL_FRAMEBUFFER,
    GL_COLOR_ATTACHMENT0,
    GL_TEXTURE_2D,
    texture,
    0
  );

  glBindTexture (GL_TEXTURE_2D, 0);

  glGenRenderbuffers (1, &depth);
  glBindRenderbuffer (GL_RENDERBUFFER, depth);

  glRenderbufferStorage (
    GL_RENDERBUFFER,
    GL_DEPTH24_STENCIL8,
    static_cast<GLsizei> (_w),
    static_cast<GLsizei> (_h)
  );

  glFramebufferRenderbuffer(
    GL_FRAMEBUFFER,
    GL_DEPTH_STENCIL_ATTACHMENT,
    GL_RENDERBUFFER,
    depth
  );

  glBindRenderbuffer (GL_RENDERBUFFER, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


Framebuffer::~Framebuffer ()
{
  glDeleteTextures (1, &texture);
  glDeleteRenderbuffers (1, &depth);
  glDeleteFramebuffers (1, &fbo);
}


void
Framebuffer::bind () const
{
  glBindFramebuffer (GL_FRAMEBUFFER, fbo);
  glViewport (0, 0, static_cast <GLsizei> (w), static_cast <GLsizei> (h));
}


void
Framebuffer::unbind () const
{
  glBindFramebuffer (GL_FRAMEBUFFER, 0);
}


void
Framebuffer::unbind (uint32_t _w, uint32_t _h) const
{
  glBindFramebuffer (GL_FRAMEBUFFER, 0);
  glViewport (0, 0, static_cast <GLsizei> (_w), static_cast <GLsizei> (_h));
}


///////////////////////////////////////////////////////////////////////////////


struct Text : sf::Text
{
  using sf::Text::Text;

  Text (const sf::Font &font, unsigned size);

  void
  write (const std::string &s);

  void
  place (float px, float py);

  void
  place (float px, float py, float mx, float my);

  sf::RectangleShape
  background () const;
};


///////////////////////////////////////////////////////////////////////////////


Text::Text (const sf::Font &font, unsigned size)
{
  setFont (font);
  setCharacterSize (size);
}


void
Text::write (const std::string &s)
{
  setString (sf::String::fromUtf8 (s.begin (), s.end ()));
}


void
Text::place (float px, float py)
{
  setPosition (px, py);
}


void
Text::place (float px, float py, float mx, float my)
{
  const auto bounds = getLocalBounds ();

  // NOTE: int is necessary to avoid ugly aliasing.
  const int x = bounds.left + bounds.width * mx;
  const int y = bounds.top + bounds.height * my;

  setOrigin (x, y);

  setPosition (px, py);
}


sf::RectangleShape
Text::background () const
{
  const auto bounds = getLocalBounds ();

  sf::RectangleShape shape;

  shape.setPosition ({ bounds.left, bounds.top });
  shape.setSize ({ bounds.width, bounds.height });

  return shape;
}


///////////////////////////////////////////////////////////////////////////////


int
main (int argc, char **argv)
{
  if (argc != 2)
    {
      std::cout << "usage: " << argv[0] << " <file>.csv" << std::endl;
      exit (1);
    }

  /////////////////////////////////////////////////////////////////////////////

  std::string file = argv[1];

  std::cout << "Importing data...\n";

  std::vector<Star> stars;

  if (!import_gaia (stars, file))
    {
      std::cerr << "Failed to import data\n";
      exit (1);
    }

  const size_t N_STARS = stars.size ();

  /////////////////////////////////////////////////////////////////////////////

  sf::VideoMode mode{ WW, WH };

  sf::ContextSettings settings{ 24, 8, 8, 3 /* major */, 3 /* minor */ };

  sf::RenderWindow window{ mode, "Gaia Explorer", sf::Style::Titlebar, settings };

  sf::VideoMode desktop = sf::VideoMode::getDesktopMode ();

  const int SW = desktop.width;
  const int SH = desktop.height;

  window.setPosition ({
    SW / 2 - WW / 2,
    SH / 2 - WH / 2
  });

  window.setVerticalSyncEnabled (VSYNC);
  window.setMouseCursorGrabbed (true);
  window.setMouseCursorVisible (SHOW_MOUSE);

  /////////////////////////////////////////////////////////////////////////////
 
  glewExperimental = GL_TRUE;

  if (glewInit () != GLEW_OK)
    {
      std::cerr << "Failed to initialize GLEW\n";
      return 1;
    }

  /////////////////////////////////////////////////////////////////////////////

  sf::Mouse::setPosition ({
    WW / 2,
    WH / 2
  }, window);

  Mouse mouse;

  mouse.update (window);

  /////////////////////////////////////////////////////////////////////////////

  sf::Font font;

  if (!font.loadFromFile ("res/SourceCodePro-Regular.ttf"))
    {
      std::cerr << "Failed to import font";
      return 1;
    }

  Text text_perf (font, 16);
  Text text_file (font, 16);
  Text text_camera (font, 16);
  Text text_speed (font, 16);

  text_file.setFillColor ({ 64, 64, 64 });
  text_camera.setFillColor ({ 64, 64, 64 });
  text_speed.setFillColor ({ 64, 96, 128 });

  text_file.write (std::to_string (N_STARS) + " stars imported\n" + file);

  /////////////////////////////////////////////////////////////////////////////

  Camera camera;

  camera.position = vector3su::ZERO;

  camera.ra  = 0;
  camera.dec = 0;
  camera.fov = 90;

  double fov_target = camera.fov;

  auto camera_speed = spatial_unit::from_ly (1);

  /////////////////////////////////////////////////////////////////////////////

  sf::VertexArray points (sf::Points);

  points.resize (N_STARS);

  for (size_t i = 0; i < N_STARS; ++i)
    points[i].color = stars[i].tint;

  /////////////////////////////////////////////////////////////////////////////

  sf::Shader shader_blur;

  shader_blur.loadFromMemory (VS_BLUR, FS_BLUR);
  shader_blur.setUniform ("u_texture", sf::Shader::CurrentTexture);
  shader_blur.setUniform ("u_texel", sf::Glsl::Vec2{ 1.0f / WW, 1.0f / WH });

  sf::Shader shader_basic_color;

  shader_basic_color.loadFromMemory (VS_BASIC_COLOR, FS_BASIC_COLOR);

  sf::Shader shader_basic_texture;

  shader_basic_texture.loadFromMemory (VS_BASIC_TEXTURE, FS_BASIC_TEXTURE);
  shader_basic_texture.setUniform ("u_texture", sf::Shader::CurrentTexture);

  /////////////////////////////////////////////////////////////////////////////

  float quad[] = {
    // position    // uv
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f,

    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f,
    -1.0f,  1.0f,  0.0f, 1.0f
  };

  constexpr size_t qv_sz = 4 * sizeof (float);

  GLuint quad_vao;
  GLuint quad_vbo;

  glGenVertexArrays (1, &quad_vao);
  glGenBuffers (1, &quad_vbo);

  glBindVertexArray (quad_vao);

  glBindBuffer (GL_ARRAY_BUFFER, quad_vbo);
  glBufferData (GL_ARRAY_BUFFER, sizeof quad, quad, GL_STATIC_DRAW);

  glEnableVertexAttribArray (0);
  glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, qv_sz, (void *)0);

  glEnableVertexAttribArray (1);
  glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, qv_sz, (void *)(2 * sizeof (float)));

  glBindVertexArray (0);

  /////////////////////////////////////////////////////////////////////////////

  struct Vertex
  {
    float x;
    float y;

    float r;
    float g;
    float b;
    float a;
  };

  GLuint vao;
  GLuint vbo;

  window.setActive(true);
  glGenVertexArrays (1, &vao);
  glGenBuffers (1, &vbo);

  glBindVertexArray (vao);

  glBindBuffer (GL_ARRAY_BUFFER, vbo);

  glBufferStorage (
    GL_ARRAY_BUFFER,
    N_STARS * sizeof (Vertex),
    nullptr,
    GL_MAP_WRITE_BIT |
    GL_MAP_PERSISTENT_BIT |
    GL_MAP_COHERENT_BIT
  );

  auto vertices = static_cast<Vertex*> (
    glMapBufferRange (
      GL_ARRAY_BUFFER, 0, N_STARS * sizeof (Vertex),
      GL_MAP_WRITE_BIT |
      GL_MAP_PERSISTENT_BIT |
      GL_MAP_COHERENT_BIT
    )
  );

  glEnableVertexAttribArray (0);
  glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, sizeof (Vertex), (void *)0);

  glEnableVertexAttribArray (1);
  glVertexAttribPointer (1, 4, GL_FLOAT, GL_FALSE, sizeof (Vertex), (void *)(2 * sizeof (float)));

  glBindVertexArray (0);

  GLsync fence = 0;

  /////////////////////////////////////////////////////////////////////////////

  Framebuffer scene (WW, WH, Framebuffer::Format::RGBA16F);
  Framebuffer blur_v (WW, WH, Framebuffer::Format::RGBA16F);
  Framebuffer blur_h (WW, WH, Framebuffer::Format::RGBA16F);

  /////////////////////////////////////////////////////////////////////////////

  bool gui = true;

  /////////////////////////////////////////////////////////////////////////////

  sf::Clock clock;

  while (window.isOpen ())
    {
      const float dt = clock.restart ().asSeconds ();

      sf::Event event;

      while (window.pollEvent (event))
        switch (event.type)
          {
          case sf::Event::Closed:
            window.close ();
            break;

          case sf::Event::KeyPressed:
            switch (event.key.code)
              {
              case sf::Keyboard::Escape:
                window.close ();
                break;
              case sf::Keyboard::F1:
                gui = !gui;
                break;
              case sf::Keyboard::Num0:
              case sf::Keyboard::Home:
                camera.position = vector3su::ZERO;
                break;
              case sf::Keyboard::Add:
                camera.exposure *= 2.0;
                break;
              case sf::Keyboard::Subtract:
                camera.exposure /= 2.0;
                break;
              case sf::Keyboard::R:
                camera_speed *= 10;
                break;
              case sf::Keyboard::F:
                camera_speed /= 10;
                break;
              case sf::Keyboard::C:
                camera_speed = spatial_unit::from_km (SOL);
                break;
              default:
                break;
              }
            break;

          case sf::Event::MouseWheelScrolled:
            if (sf::Mouse::isButtonPressed (sf::Mouse::Right))
              {
                if (event.mouseWheelScroll.delta > 0)
                  fov_target /= 1.2;

                if (event.mouseWheelScroll.delta < 0)
                  fov_target *= 1.2;

                fov_target = clamp (fov_target, 0.0, 90 * std::pow (1.2, 2));
              }
            else
              {
                if (event.mouseWheelScroll.delta > 0)
                  // Magic = (2^63 - 1) / 1.5
                  if (camera_speed < spatial_unit::from_km (6148914691236516864))
                    camera_speed *= 1.5;

                if (event.mouseWheelScroll.delta < 0)
                  camera_speed /= 1.5;
              }

            break;

          default:
            break;
          }

      camera.fov += (fov_target - camera.fov) * (1.0f - std::exp (-8 * dt));

      /////////////////////////////////////////////////////////////////////////

      mouse.update (window);

      camera.ra  -= mouse.delta.x * 0.1 * (camera.fov / 90.0);
      camera.dec -= mouse.delta.y * 0.1 * (camera.fov / 90.0);

      camera.ra  = camera.ra - std::floor(camera.ra / 360.0) * 360.0;
      camera.dec = clamp (camera.dec, -90.0, 90.0);

      const double r_rad = RAD (camera.ra);
      const double d_rad = RAD (camera.dec);

      const double r_sin = std::sin (r_rad);
      const double r_cos = std::cos (r_rad);

      const double d_sin = std::sin (d_rad);
      const double d_cos = std::cos (d_rad);

      const double fx =  d_cos * r_cos;
      const double fy =  d_cos * r_sin;
      const double fz =  d_sin;

      const double rx =  r_sin;
      const double ry = -r_cos;
      const double rz =  0.0;

      const double ux = -d_sin * r_cos;
      const double uy = -d_sin * r_sin;
      const double uz =  d_cos;

      const auto move_speed = camera_speed * dt;

      if (sf::Keyboard::isKeyPressed (sf::Keyboard::W))
        {
          camera.position.x += fx * move_speed;
          camera.position.y += fy * move_speed;
          camera.position.z += fz * move_speed;
        }

      if (sf::Keyboard::isKeyPressed (sf::Keyboard::S))
        {
          camera.position.x -= fx * move_speed;
          camera.position.y -= fy * move_speed;
          camera.position.z -= fz * move_speed;
        }

      if (sf::Keyboard::isKeyPressed (sf::Keyboard::D))
        {
          camera.position.x += rx * move_speed;
          camera.position.y += ry * move_speed;
          camera.position.z += rz * move_speed;
        }

      if (sf::Keyboard::isKeyPressed (sf::Keyboard::A))
        {
          camera.position.x -= rx * move_speed;
          camera.position.y -= ry * move_speed;
          camera.position.z -= rz * move_speed;
        }

      if (sf::Keyboard::isKeyPressed (sf::Keyboard::Space))
        {
          camera.position.x += ux * move_speed;
          camera.position.y += uy * move_speed;
          camera.position.z += uz * move_speed;
        }

      if (sf::Keyboard::isKeyPressed (sf::Keyboard::LControl))
        {
          camera.position.x -= ux * move_speed;
          camera.position.y -= uy * move_speed;
          camera.position.z -= uz * move_speed;
        }

      /////////////////////////////////////////////////////////////////////////

      window.setActive (true);

      Cache cache (camera);

      const double cx = camera.position.x.to_pc ();
      const double cy = camera.position.y.to_pc ();
      const double cz = camera.position.z.to_pc ();

      const double brightness_scale = camera.exposure * cache.inv_half_h;

      if (fence)
        {
          glClientWaitSync (fence, 0, 0);
          glDeleteSync (fence);
          fence = 0;
        }

      for (size_t i = 0; i < N_STARS; ++i)
        {
          const auto &star = stars[i];

          auto &vertex = vertices[i];

          double px, py;

          double dx = star.x - cx;
          double dy = star.y - cy;
          double dz = star.z - cz;

          if (!project (dx, dy, dz, cache, px, py))
            {
              vertex.a = 0;
              continue;
            }

          vertex.x = px;
          vertex.y = py;

          double d2 = dx * dx + dy * dy + dz * dz;

          double brightness = brightness_scale * (star.lum / d2);

          vertex.r = static_cast<float> (star.tint.r) / 255.0f;
          vertex.g = static_cast<float> (star.tint.g) / 255.0f;
          vertex.b = static_cast<float> (star.tint.b) / 255.0f;
          vertex.a = static_cast<float> (clamp (brightness, 0.0, 1.0 / 16.0));
        }

      /////////////////////////////////////////////////////////////////////////

      glEnable (GL_BLEND);
      glBlendFunc (GL_SRC_ALPHA, GL_ONE);

      // Scene
      scene.bind ();

      glClearColor (0, 0, 0, 1);
      glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      sf::Shader::bind (&shader_basic_color);

      glBindVertexArray (vao);
      glDrawArrays (GL_POINTS, 0, N_STARS);
      glBindVertexArray (0);

      scene.unbind ();

      // Horizontal blur
      blur_h.bind ();

      glClearColor (0, 0, 0, 1);
      glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      shader_blur.setUniform ("u_direction", sf::Glsl::Vec2{ 1, 0 });
      sf::Shader::bind (&shader_blur);

      glActiveTexture (GL_TEXTURE0);
      glBindTexture (GL_TEXTURE_2D, scene.texture);

      glBindVertexArray (quad_vao);
      glDrawArrays (GL_TRIANGLES, 0, 6);
      glBindVertexArray (0);

      blur_h.unbind ();

      // Vertical blur
      blur_v.bind ();

      glClearColor (0, 0, 0, 1);
      glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      shader_blur.setUniform ("u_direction", sf::Glsl::Vec2{ 0, 1 });
      sf::Shader::bind (&shader_blur);

      glActiveTexture (GL_TEXTURE0);
      glBindTexture (GL_TEXTURE_2D, blur_h.texture);

      glBindVertexArray (quad_vao);
      glDrawArrays (GL_TRIANGLES, 0, 6);
      glBindVertexArray (0);

      blur_v.unbind ();

      // Fullscreen quad
      glClearColor (16.0 / 255.0, 16.0 / 255.0, 16.0 / 255.0, 1);
      glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      sf::Shader::bind (&shader_basic_texture);

      glActiveTexture (GL_TEXTURE0);
      glBindTexture (GL_TEXTURE_2D, blur_v.texture);

      glBindVertexArray (quad_vao);
      glDrawArrays (GL_TRIANGLES, 0, 6);
      glBindVertexArray (0);

      if (fence)
        glDeleteSync (fence);

      fence = glFenceSync (GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

      /////////////////////////////////////////////////////////////////////////

      window.resetGLStates ();

      if (gui)
        {
          if (dt < 1.0 / 60.0)
            text_perf.setFillColor(sf::Color{ 32, 128, 32 });
          else if (dt < 1.0 / 30.0)
            text_perf.setFillColor(sf::Color{ 128, 64, 32 });
          else
            text_perf.setFillColor(sf::Color{ 128, 32, 32 });

          text_perf.write (to_human (dt) + "s " + fmt (1.0f / dt) + " fps");
          text_perf.place (10, 10);
          window.draw (text_perf);

          // ------------------------------------------------------------------

          text_file.place (10, WH - 10, 0, 1);
          window.draw (text_file);

          // ------------------------------------------------------------------

          text_camera.write (
            "Ra "  + fmt (camera.ra)  + "°\n" +
            "Dec " + fmt (camera.dec) + "°\n" +
            "FOV " + fmt (camera.fov) + "°"
          );
          text_camera.place (WW - 10, WH - 10, 1, 1);
          window.draw (text_camera);

          // ------------------------------------------------------------------

          text_speed.write (
            to_human (camera_speed) + "/s (" +
            to_human (camera_speed.to_km () / SOL) + "c)"
          );
          text_speed.place (WW / 2, WH - 24, 0.5, 1);
          window.draw (text_speed);

          // ------------------------------------------------------------------

          sf::RectangleShape crosshair;

          crosshair.setSize ({ 4.0f, 4.0f });
          crosshair.setPosition (WW / 2.0f - 2.0f, WH / 2.0f - 2.0f);
          crosshair.setFillColor (sf::Color::Transparent);
          crosshair.setOutlineColor (sf::Color{ 255, 224, 64 });
          crosshair.setOutlineThickness (1);

          window.draw (crosshair);
        }

      /////////////////////////////////////////////////////////////////////////

      window.display ();
    }

  glDeleteVertexArrays (1, &quad_vao);
  glDeleteBuffers (1, &quad_vbo);

  glDeleteVertexArrays (1, &vao);
  glDeleteBuffers (1, &vbo);

  // delete[] vertices;

  std::cout << "Goodbye.\n";
}


