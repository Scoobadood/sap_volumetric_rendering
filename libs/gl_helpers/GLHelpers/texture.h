#ifndef GL_HELPERS_TEXTURE_H
#define GL_HELPERS_TEXTURE_H

#include <string>

#include "stb_image.h"

class Texture {
 public:
  explicit Texture(const std::string &file_name);
  Texture(uint32_t width, uint32_t height);

  ~Texture();

  void BindToTextureUnit(uint32_t tu) const;

  static std::shared_ptr<Texture> FromImageFile(const std::string &file_name);

  void SetImageData(uint8_t *image_data);

  inline uint32_t Id() const { return texture_id_; }

 private:
  Texture();

  uint32_t width_;
  uint32_t height_;

  void SetImage(uint8_t *image_data, int32_t width, int32_t height, int32_t channels);

  uint32_t texture_id_;
  static int32_t MAX_TEXTURE_UNIT_ID;
};

#endif
