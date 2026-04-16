#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstddef> 


///////////////////////////////////////////////////////////////////////////////


constexpr bool VSYNC      = false;
constexpr bool SHOW_MOUSE = false;


///////////////////////////////////////////////////////////////////////////////


static int WW;
static int WH;
static double ASPECT;


///////////////////////////////////////////////////////////////////////////////


static constexpr double PI   = 3.14159265358979323846;
static constexpr double PI_2 = 1.57079632679489661923;
static constexpr double PI_4 = 0.78539816339744830962;

#define RAD(a) ((a) * (PI / 180.0))
#define DEG(a) ((a) / (PI / 180.0))


// ----------------------------------------------------------------------------


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


static constexpr double PC_PER_LY = 0.30660139383437;
static constexpr double LY_PER_PC = 3.2615637766481;

static constexpr double PC_PER_AU = 4.8481368118674e-6;
static constexpr double AU_PER_PC = 206264.80621425;

static constexpr double PC_PER_KM = 3.2407792899604e-14;
static constexpr double KM_PER_PC = 30856775810000;

static constexpr double SOL_KM_S = 299'792.458;
static constexpr double SOL_PC_S = SOL_KM_S / KM_PER_PC;


constexpr double
parsec_from_km (double km)
{
  return km * PC_PER_KM;
}


constexpr double
parsec_to_km (double pc)
{
  return pc * KM_PER_PC;
}


constexpr double
parsec_from_au (double au)
{
  return au * PC_PER_AU;
}


constexpr double
parsec_to_au (double pc)
{
  return pc * AU_PER_PC;
}


constexpr double
parsec_from_ly (double ly)
{
  return ly * PC_PER_LY;
}


constexpr double
parsec_to_ly (double pc)
{
  return pc * LY_PER_PC;
}


///////////////////////////////////////////////////////////////////////////////


struct vec3
{
  double x;
  double y;
  double z;

  double
  length () const;

  vec3
  operator+ (vec3 b) const;

  vec3
  operator- (vec3 b) const;

  vec3
  operator* (double s) const;

  vec3
  operator/ (double s) const;

  vec3 &
  operator+= (vec3 b);

  vec3 &
  operator-= (vec3 b);
};


// ----------------------------------------------------------------------------


double
vec3::length () const
{
  return std::sqrt (x * x + y * y + z * z);
}


vec3
vec3::operator+ (vec3 b) const
{
  return {
    x + b.x,
    y + b.y,
    z + b.z
  };
}


vec3
vec3::operator- (vec3 b) const
{
  return {
    x - b.x,
    y - b.y,
    z - b.z
  };
}


vec3
vec3::operator* (double s) const
{
  return {
    x * s,
    y * s,
    z * s
  };
}


vec3
vec3::operator/ (double s) const
{
  const double inv_s = 1.0 / s;

  return {
    x * inv_s,
    y * inv_s,
    z * inv_s
  };
}


vec3 &
vec3::operator+= (vec3 b)
{
  x += b.x;
  y += b.y;
  z += b.z;
  return *this;
}


vec3 &
vec3::operator-= (vec3 b)
{
  x -= b.x;
  y -= b.y;
  z -= b.z;
  return *this;
}


inline double
dot (vec3 a, vec3 b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}


inline vec3
normalize (vec3 a)
{
  return a / std::sqrt (dot (a, a));
}


inline vec3
cross (vec3 a, vec3 b)
{
  return {
    a.y * b.z - a.z * b.y,
    a.z * b.x - a.x * b.z,
    a.x * b.y - a.y * b.x
  };
}


///////////////////////////////////////////////////////////////////////////////


static std::string
to_human_round (double d, int n)
{
  char buffer[256];

  std::snprintf (buffer, sizeof buffer, "%.*f", n, d);

  return buffer;
};


std::string
separate_with (std::string s, char sep = ',')
{
  int insert_pos = s.length () - 3;

  while (insert_pos > 0)
    {
      s.insert(insert_pos, 1, sep);
      insert_pos -= 3;
    }

  return s;
}


std::string
separate (int64_t n, char sep = ',')
{
  return separate_with (std::to_string (n), sep);
}


std::string
separate (uint64_t n, char sep = ',')
{
  return separate_with (std::to_string (n), sep);
}


std::string
separate (double d, int n, char sep = ',')
{
  const std::string s = to_human_round (d, n);

  auto pos = s.find ('.');

  if (pos == std::string::npos)
    return separate_with (s, sep);

  const std::string integer = s.substr (0, pos);
  const std::string fraction = s.substr (pos);

  return separate_with (integer, sep) + fraction;
}


std::string
to_human (double x)
{
  if (x == 0)
    return separate (x, 1);

  const auto ax = std::abs (x);

  if (ax >= 1e12)
    return separate (x * 1e-12, 1) + "T";

  if (ax >= 1e9)
    return separate (x * 1e-9, 1) + "G";

  if (ax >= 1e6)
    return separate (x * 1e-6, 1) + "M";

  if (ax >= 1e3)
    return separate (x * 1e-3, 1) + "k";

  if (ax >= 1)
    return separate (x, 1);

  if (ax >= 1e-3)
    return separate (x * 1e3, 1) + "m";

  if (ax >= 1e-6)
    return separate (x * 1e6, 1) + "µ";

  if (ax >= 1e-9)
    return separate (x * 1e9, 1) + "n";

  return separate (x * 1e12, 1) + "p";
}


std::string
to_human_parsec (double a)
{
  const double ax = std::abs (a);

  if (ax >= 0.1 * PC_PER_LY)
    return separate (parsec_to_ly (a), 1) + " ly";

  if (ax >= 0.1 * PC_PER_AU)
    return separate (parsec_to_au (a), 1) + " AU";

  return separate (parsec_to_km (a), 0) + " km";
}


///////////////////////////////////////////////////////////////////////////////


void
temperature_to_color (double temp, float &r, float &g, float &b)
{
  double T = temp / 100.0;

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

  Gaia_Star (const std::string &line);
};


// ----------------------------------------------------------------------------


Gaia_Star::Gaia_Star (const std::string &line)
{
  const char *p = line.c_str ();

  char *end;

  source_id = std::strtoll (p, &end, 10);
  p = end + 1;

  ra = std::strtod (p, &end);
  p = end + 1;

  dec = std::strtod (p, &end);
  p = end + 1;

  parallax = std::strtod (p, &end);
  p = end + 1;

  lum_flame = std::strtod (p, &end);
  p = end + 1;

  teff_gspphot = std::strtod (p, &end);
}


///////////////////////////////////////////////////////////////////////////////


bool
import_gaia (std::vector<Gaia_Star> &stars, const std::string &path)
{
  std::ifstream file (path);

  if (!file.is_open ())
    return false;

  stars.reserve (3'000'000);

  std::string line;

  std::getline (file, line); // Header

  while (std::getline (file, line))
    stars.emplace_back (/* Gaia_Star{} */ line);

  return !file.bad ();
}


///////////////////////////////////////////////////////////////////////////////


struct Star
{
  float p[3];

  float lum;

  float t[3];

  Star () = default;

  Star (const Gaia_Star &gaia_star);
};


// ----------------------------------------------------------------------------


Star::Star (const Gaia_Star &gaia_star)
{
  double r_rad = RAD (gaia_star.ra);
  double d_rad = RAD (gaia_star.dec);

  double distance_pc = 1000.0 / gaia_star.parallax;

  p[0] = distance_pc * std::cos (d_rad) * std::cos (r_rad);
  p[1] = distance_pc * std::cos (d_rad) * std::sin (r_rad);
  p[2] = distance_pc * std::sin (d_rad);

  lum  = gaia_star.lum_flame;

  float r, g, b;

  temperature_to_color (gaia_star.teff_gspphot, r, g, b);

  t[0] = r / 255.0f;
  t[1] = g / 255.0f;
  t[2] = b / 255.0f;
}


///////////////////////////////////////////////////////////////////////////////


struct Camera
{
  vec3 position{
    0.0 /* pc */,
    0.0 /* pc */,
    0.0 /* pc */
  };

  vec3 forward { 1,  0, 0 };
  vec3 right   { 0, -1, 0 };
  vec3 up      { 0,  0, 1 };

  double fov  /* degree */;

  double exposure{ 1.0 / 16.0 };
};


///////////////////////////////////////////////////////////////////////////////


struct Mouse
{
  sf::Vector2i position{ 0, 0 };
  sf::Vector2i previous{ 0, 0 };

  sf::Vector2i delta{ 0, 0 };

  void
  update (const sf::RenderWindow &window);
};


// ----------------------------------------------------------------------------


void
Mouse::update (const sf::RenderWindow &window)
{
  position = sf::Mouse::getPosition (window);

  delta = position - previous;

  static const int X_MIN = 0;
  static const int Y_MIN = 0;
  static const int X_MAX = WW - 1;
  static const int Y_MAX = WH - 1;

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


///////////////////////////////////////////////////////////////////////////////


constexpr const char *VS_PROJECT = R"(
#version 330 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in float a_lum;
layout (location = 2) in vec3 a_tint;

uniform vec3 u_camera_position;
uniform vec3 u_camera_forward;
uniform vec3 u_camera_right;
uniform vec3 u_camera_up;

uniform float u_inv_half_h;
uniform float u_inv_half_w;
uniform float u_brightness_scale;

out vec4 v_color;

void
main ()
{
  vec3 d = a_position - u_camera_position;

  float cz = dot (d, u_camera_forward);

  if (cz <= 0.0)
    {
      v_color = vec4 (a_tint.rgb, 0);
      return;
    }

  float inv_cz = 1.0 / cz;

  float px = dot (d, u_camera_right) * inv_cz * u_inv_half_w;
  float py = dot (d, u_camera_up)    * inv_cz * u_inv_half_h;

  gl_Position = vec4 (px, py, 0.0, 1.0);

  float d2 = dot (d, d);
  float brightness = u_brightness_scale * (a_lum / d2);

  v_color = vec4(a_tint.rgb, clamp (brightness, 0.0, 1.0 / 16.0));
}
)";


constexpr const char *FS_PROJECT = R"(
#version 330 core

in vec4 v_color;

out vec4 f_color;

void
main ()
{
  f_color = v_color;
}
)";


// ----------------------------------------------------------------------------


constexpr const char *VS_TEXTURE = R"(
#version 330 core

layout (location = 0) in vec2 a_position;
layout (location = 1) in vec2 a_uv;

out vec2 v_uv;

void
main ()
{
  gl_Position = vec4 (a_position, 0.0, 1.0);

  v_uv = a_uv;
}
)";


constexpr const char *FS_TEXTURE = R"(
#version 330 core

uniform sampler2D u_texture;

in vec2 v_uv;

out vec4 f_color;

void
main ()
{
  f_color = texture2D (u_texture, v_uv);
}
)";


// ----------------------------------------------------------------------------


constexpr const char *VS_BLUR = R"(
#version 330 core

layout (location = 0) in vec2 a_position;

void
main ()
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

void
main ()
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
  backdrop () const;
};


// ----------------------------------------------------------------------------


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
Text::backdrop () const
{
  constexpr float p = 5;

  const auto bounds = getGlobalBounds ();

  sf::RectangleShape shape;

  shape.setPosition ({ bounds.left - p, bounds.top - p });
  shape.setSize ({ bounds.width + p * 2, bounds.height + p * 2 });
  shape.setFillColor (sf::Color{ 0, 0, 0, 128 });

  sf::Color outline_color = getFillColor ();

  outline_color.r /= 2;
  outline_color.g /= 2;
  outline_color.b /= 2;
  outline_color.a = 128;

  shape.setOutlineColor (outline_color);
  shape.setOutlineThickness (2);

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

  std::vector<Gaia_Star> gaia_stars;

  if (!import_gaia (gaia_stars, file))
    {
      std::cerr << "Failed to import data\n";
      exit (1);
    }

  const size_t N_STARS = gaia_stars.size ();

  /////////////////////////////////////////////////////////////////////////////

  sf::VideoMode desktop = sf::VideoMode::getDesktopMode ();

  WW = desktop.width;
  WH = desktop.height;

  ASPECT = static_cast<double> (WW) / static_cast<double> (WH);

  sf::VideoMode mode{
    static_cast<unsigned> (WW),
    static_cast<unsigned> (WH)
  };

  sf::ContextSettings settings{
    24,
    8,
    8,
    3 /* major */,
    3 /* minor */
  };

  sf::RenderWindow window{
    mode,
    "Gaia Explorer",
    sf::Style::Fullscreen,
    settings
  };

  /*
  window.setPosition ({
    SW / 2 - WW / 2,
    SH / 2 - WH / 2
  });
  */

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

  text_file.setFillColor ({ 128, 128, 128 });
  text_camera.setFillColor ({ 128, 128, 128 });
  text_speed.setFillColor ({ 128, 196, 255 });

  text_file.write (separate (N_STARS) + " stars\n" + file);

  /////////////////////////////////////////////////////////////////////////////

  Camera camera;

  camera.fov = 90;

  double fov_target = camera.fov;

  double camera_speed = parsec_from_ly (1.0);

  /////////////////////////////////////////////////////////////////////////////

  sf::Shader shader_project;

  shader_project.loadFromMemory (VS_PROJECT, FS_PROJECT);

  sf::Shader shader_blur;

  shader_blur.loadFromMemory (VS_BLUR, FS_BLUR);
  shader_blur.setUniform ("u_texture", sf::Shader::CurrentTexture);
  shader_blur.setUniform ("u_texel", sf::Glsl::Vec2{ 1.0f / WW, 1.0f / WH });

  sf::Shader shader_texture;

  shader_texture.loadFromMemory (VS_TEXTURE, FS_TEXTURE);
  shader_texture.setUniform ("u_texture", sf::Shader::CurrentTexture);

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

  window.setActive (true);

  GLuint vao;
  GLuint vbo;

  glGenVertexArrays (1, &vao);
  glGenBuffers (1, &vbo);

  glBindVertexArray (vao);

  glBindBuffer (GL_ARRAY_BUFFER, vbo);

  Star *stars = new Star[N_STARS];

  for (size_t i = 0; i < N_STARS; ++i)
    stars[i] = Star (gaia_stars[i]);

  glBufferData (GL_ARRAY_BUFFER, N_STARS * sizeof (Star), stars, GL_STATIC_DRAW);

  delete[] stars;

  glEnableVertexAttribArray (0);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, sizeof (Star), (void *)offsetof (Star, p));

  glEnableVertexAttribArray (1);
  glVertexAttribPointer (1, 1, GL_FLOAT, GL_FALSE, sizeof (Star), (void *)offsetof (Star, lum));

  glEnableVertexAttribArray (2);
  glVertexAttribPointer (2, 3, GL_FLOAT, GL_FALSE, sizeof (Star), (void *)offsetof (Star, t));

  glBindVertexArray (0);

  /////////////////////////////////////////////////////////////////////////////

  Framebuffer scene (WW, WH, Framebuffer::Format::RGBA16F);
  Framebuffer blur_v (WW, WH, Framebuffer::Format::RGBA16F);
  Framebuffer blur_h (WW, WH, Framebuffer::Format::RGBA16F);

  /////////////////////////////////////////////////////////////////////////////

  bool gui = true;

  sf::RectangleShape crosshair;

  crosshair.setSize ({ 4.0f, 4.0f });
  crosshair.setPosition (WW / 2.0f - 2.0f, WH / 2.0f - 2.0f);
  crosshair.setFillColor (sf::Color::Transparent);
  crosshair.setOutlineColor (sf::Color{ 255, 224, 64 });
  crosshair.setOutlineThickness (1);

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
                camera.position = { 0, 0, 0 };
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
                camera_speed = SOL_PC_S;
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

      // Roll
      {
        const double SENSITIVITY = 0.1 * (/* camera.fov */ 90.0 / 90.0);

        double r_rad = 0;

        if (sf::Mouse::isButtonPressed (sf::Mouse::Middle))
          r_rad += RAD (mouse.delta.x * SENSITIVITY);

        if (sf::Keyboard::isKeyPressed (sf::Keyboard::Q))
          r_rad -= RAD (180) * dt;

        if (sf::Keyboard::isKeyPressed (sf::Keyboard::E))
          r_rad += RAD (180) * dt;

        const double sr = std::sin (r_rad);
        const double cr = std::cos (r_rad);

        const vec3 new_right = normalize (camera.right * cr - camera.up    * sr);
        const vec3 new_up    = normalize (camera.up    * cr + camera.right * sr);

        camera.right = new_right;
        camera.up    = new_up;
      }

      // Yaw, Pitch
      {
        const double SENSITIVITY = 0.1 * (camera.fov / 90.0);

        double y_rad = 0;
        double p_rad = 0;

        if (!sf::Mouse::isButtonPressed (sf::Mouse::Middle))
          {
            y_rad += RAD (mouse.delta.x * SENSITIVITY);
            p_rad += RAD (mouse.delta.y * SENSITIVITY);
          }

        const double sy = std::sin (y_rad);
        const double cy = std::cos (y_rad);

        const double sp = std::sin (p_rad);
        const double cp = std::cos (p_rad);

        {
          const vec3 new_forward = normalize (camera.forward * cy + camera.right   * sy);
          const vec3 new_right   = normalize (camera.right   * cy - camera.forward * sy);

          camera.forward = new_forward;
          camera.right   = new_right;
        }

        {
          const vec3 new_forward = normalize (camera.forward * cp - camera.up      * sp);
          const vec3 new_up      = normalize (camera.up      * cp + camera.forward * sp);

          camera.forward = new_forward;
          camera.up      = new_up;
        }
      }

      camera.forward = normalize (camera.forward);
      camera.right   = normalize (cross (camera.forward, camera.up));
      camera.up      = normalize (cross (camera.right,   camera.forward));

      {
        const auto MOVE_SPEED = camera_speed * dt;

        if (sf::Keyboard::isKeyPressed (sf::Keyboard::W))
          camera.position += camera.forward * MOVE_SPEED;
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::S))
          camera.position -= camera.forward * MOVE_SPEED;

        if (sf::Keyboard::isKeyPressed (sf::Keyboard::D))
          camera.position += camera.right * MOVE_SPEED;
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::A))
          camera.position -= camera.right * MOVE_SPEED;

        if (sf::Keyboard::isKeyPressed (sf::Keyboard::Space))
          camera.position += camera.up * MOVE_SPEED;
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::LControl))
          camera.position -= camera.up * MOVE_SPEED;
      }

      /////////////////////////////////////////////////////////////////////////

      window.setActive (true);

      shader_project.setUniform (
        "u_camera_position",
        sf::Glsl::Vec3{
          static_cast<float> (camera.position.x),
          static_cast<float> (camera.position.y),
          static_cast<float> (camera.position.z)
        }
      );

      shader_project.setUniform (
        "u_camera_forward",
        sf::Glsl::Vec3{
          static_cast<float> (camera.forward.x),
          static_cast<float> (camera.forward.y),
          static_cast<float> (camera.forward.z)
        }
      );

      shader_project.setUniform (
        "u_camera_right",
        sf::Glsl::Vec3{
          static_cast<float> (camera.right.x),
          static_cast<float> (camera.right.y),
          static_cast<float> (camera.right.z)
        }
      );

      shader_project.setUniform (
        "u_camera_up",
        sf::Glsl::Vec3{
          static_cast<float> (camera.up.x),
          static_cast<float> (camera.up.y),
          static_cast<float> (camera.up.z)
        }
      );

      const float half_h = std::tan (RAD (camera.fov * 0.5));
      const float half_w = half_h * ASPECT;

      const float inv_half_h = 1.0f / half_h;
      const float inv_half_w = 1.0f / half_w;

      shader_project.setUniform ("u_inv_half_h", inv_half_h);
      shader_project.setUniform ("u_inv_half_w", inv_half_w);
      shader_project.setUniform ("u_brightness_scale", (float)camera.exposure * inv_half_h);

      /////////////////////////////////////////////////////////////////////////

      glEnable (GL_BLEND);
      glBlendFunc (GL_SRC_ALPHA, GL_ONE);

      // Scene
      scene.bind ();

      glClearColor (0, 0, 0, 1);
      glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      sf::Shader::bind (&shader_project);

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

      sf::Shader::bind (&shader_texture);

      glActiveTexture (GL_TEXTURE0);
      glBindTexture (GL_TEXTURE_2D, blur_v.texture);

      glBindVertexArray (quad_vao);
      glDrawArrays (GL_TRIANGLES, 0, 6);
      glBindVertexArray (0);

      /////////////////////////////////////////////////////////////////////////

      window.resetGLStates ();

      if (gui)
        {
          if (dt < 1.0 / 60.0)
            text_perf.setFillColor (sf::Color{ 32, 196, 32 });
          else if (dt < 1.0 / 30.0)
            text_perf.setFillColor (sf::Color{ 196, 128, 32 });
          else
            text_perf.setFillColor (sf::Color{ 196, 32, 32 });

          text_perf.write (to_human (dt) + "s ≈ " + to_human_round (1.0f / dt, 1) + " FPS");
          text_perf.place (10, 10, 0, 0);
          window.draw (text_perf.backdrop ());
          window.draw (text_perf);

          // ------------------------------------------------------------------

          text_file.place (10, WH - 10, 0, 1);
          window.draw (text_file.backdrop ());
          window.draw (text_file);

          // ------------------------------------------------------------------

          std::string orientation = "";

          if (camera.position.length () <= 1 * PC_PER_AU)
            {
              const vec3 f = normalize(camera.forward);

              const double x = f.x;
              const double y = f.y;
              const double z = f.z;

              double r = std::atan2 (y, x);
              double d = std::asin (z);

              if (r < 0)
                r += 2.0 * M_PI;

              orientation =
                "Ra  " + to_human_round (DEG (r), 1) + "°\n" +
                "Dec " + to_human_round (DEG (d), 1) + "°\n";
            }

          text_camera.write (
            orientation +
            "FOV " + to_human_round (camera.fov, 1) + "°"
          );
          text_camera.place (WW - 10, WH - 10, 1, 1);
          window.draw (text_camera.backdrop ());
          window.draw (text_camera);

          // ------------------------------------------------------------------

          text_speed.write (
            to_human_parsec (camera_speed) + "/s ≈ " + to_human (camera_speed / SOL_PC_S) + "c"
          );
          text_speed.place (WW / 2.0f, WH - 10, 0.5, 1);

          window.draw (text_speed.backdrop ());
          window.draw (text_speed);

          // ------------------------------------------------------------------

          window.draw (crosshair);
        }

      /////////////////////////////////////////////////////////////////////////

      window.display ();
    }

  glDeleteVertexArrays (1, &quad_vao);
  glDeleteBuffers (1, &quad_vbo);

  glDeleteVertexArrays (1, &vao);
  glDeleteBuffers (1, &vbo);

  std::cout << "Goodbye.\n";
}


