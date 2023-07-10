#include "RayRenderer.h"

#include <random>

namespace Raytracing
{
  RayRenderer::RayRenderer() : m_ViewportWidth(800), m_ViewportHeight(600)
  {
    m_ImageData = new uint32_t[m_ViewportWidth * m_ViewportHeight];
    for (size_t i = 0; i < m_ViewportWidth * m_ViewportHeight; i++)
    {
      m_ImageData[i] = Phoenix::Image::VecToRgba(glm::vec4(.1f, .4f, .1f, 1.f));
    }
    m_FinalImage =
      Phoenix::Image::Create(m_ViewportWidth, m_ViewportHeight, Phoenix::Format::RGBA, m_ImageData);
  }

  RayRenderer::~RayRenderer()
  {
    delete m_FinalImage;
    delete[] m_ImageData;
  }

  void RayRenderer::Render(const Scene& scene)
  {
    for (size_t y = 0; y < m_ViewportHeight; y++)
    {
      for (size_t x = 0; x < m_ViewportWidth; x++)
      {
        glm::vec2 remappedCoords = {(float)x / (float)m_FinalImage->GetWidth(),
                                    (float)y / (float)m_FinalImage->GetHeight()};
        remappedCoords = remappedCoords * 2.0f - 1.0f;
        auto colour = PixelColour(remappedCoords, scene);
        colour = glm::clamp(colour, glm::vec4(0.f), glm::vec4(1.f));
        m_ImageData[x + y * m_ViewportWidth] = Phoenix::Image::VecToRgba(colour);
      }
    }

    m_FinalImage->SetData(m_ImageData);
  }

  void RayRenderer::Resize()
  {
    if (m_FinalImage->GetWidth() != m_ViewportWidth ||
        m_FinalImage->GetHeight() != m_ViewportHeight)
    {
      m_FinalImage->Resize(m_ViewportWidth, m_ViewportHeight);
    }

    delete[] m_ImageData;
    m_ImageData = new uint32_t[m_ViewportWidth * m_ViewportHeight];
  }

  void RayRenderer::SaveImage(const std::string& fileName)
  {
    m_FinalImage->Save(fileName);
  }

  void RayRenderer::OnImGuiUpdate()
  {
    m_ViewportWidth = ImGui::GetContentRegionAvail().x;
    m_ViewportHeight = ImGui::GetContentRegionAvail().y;

    m_FinalImage->ImGuiBind();
  }

  glm::vec4 RayRenderer::PixelColour(glm::vec2 coords, const Scene& scene)
  {
    // TODO currently doesnt handle aspect ratio
    Ray ray;

    glm::vec4 color(0.f);

    // TODO cleanup this code

    ray.origin = scene.GetCamera()->GetPosition();
    if (scene.GetCamera()->GetAntiAliasing())
    {
      // TODO move this somewhere else, may help with performance
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_real_distribution<double> dist(0.0, 0.001);
      ray.dir = glm::normalize(glm::vec3(coords.x + dist(gen), coords.y + dist(gen), -1.f));
      for (int i = 0; i < 25; i++)
      {
        // return scene.ShootRay(ray);
        color += scene.ShootRay(ray);
      }
    }
    else
    {
      ray.dir = glm::normalize(glm::vec3(coords.x, coords.y, -1.f));
      color += scene.ShootRay(ray);
    }

    if (scene.GetCamera()->GetAntiAliasing())
    {
      return color / 25.f;
    }
    return color;
  }
}
