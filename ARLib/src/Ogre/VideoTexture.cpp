#include "OGRE/OgreTextureManager.h"
#include "OGRE/OgreHardwarePixelBuffer.h"
#include "ARLib/Ogre/VideoTexture.h"

namespace ARLib {
	VideoTexture::VideoTexture(const Ogre::String& textureName, const Ogre::String& undistortionMapTextureName, int cameraNumber, const char *ocamModelParametersFilename, float distance)
		: player(cameraNumber, ocamModelParametersFilename, distance)
	{
		texture = Ogre::TextureManager::getSingleton().createManual(
			textureName,
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			Ogre::TEX_TYPE_2D,
			player.getVideoWidth(), player.getVideoHeight(),
			0,
			Ogre::PF_BYTE_BGR,
			Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

		// load undistortion texture
		float *undistortionMapXY = new float[2 * player.getVideoWidth() * player.getVideoHeight()];
		if (undistortionMapXY)
		{
			undistortionMapTexture = Ogre::TextureManager::getSingleton().createManual(
				undistortionMapTextureName,
				Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
				Ogre::TEX_TYPE_2D,
				player.getVideoWidth(), player.getVideoHeight(),
				0,
				Ogre::PF_FLOAT16_GR,
				Ogre::TU_STATIC_WRITE_ONLY);

			player.calculateUndistortionMap(undistortionMapXY);
			Ogre::PixelBox pb(player.getVideoWidth(), player.getVideoHeight(), 1, Ogre::PF_FLOAT32_GR, undistortionMapXY);
			undistortionMapTexture->getBuffer()->blitFromMemory(pb);
			delete[] undistortionMapXY;
		}
	}

	VideoTexture::~VideoTexture()
	{
		// TODO: release textures?
	}

	void VideoTexture::update()
	{
		void *data = player.update();
		if(data)
			texture->getBuffer()->blitFromMemory(Ogre::PixelBox(player.getVideoWidth(), player.getVideoHeight(), 1, Ogre::PF_BYTE_BGR, data));
	}
}; // ARLib namespace
