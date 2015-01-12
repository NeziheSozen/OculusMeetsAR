#include "webcamstate.h"

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif
#include <stdint.h>

#include <OGRE/OgreTextureManager.h>
#include <OGRE/OgreHardwarePixelBuffer.h>
#include <OGRE/OgreResourceGroupManager.h>
#include <OGRE/OgreStringConverter.h>

namespace webcam{
	webcamstate::webcamstate()
	{
		sample = new BYTE[2*1280*960];
		CCapture::CreateInstance(&cap);
	}


	webcamstate::~webcamstate()
	{
	}

	HRESULT webcamstate::update()
	{
		//SYNC??
		HRESULT check=E_FAIL;
		sample = cap->getLastImagesample(&check);
		if(SUCCEEDED(check)){
			if (mTexture.isNull())
			{
				static int i = 0;
				mTexture = Ogre::TextureManager::getSingleton().createManual(
					"ffmpeg/VideoTexture" + Ogre::StringConverter::toString(++i),
					Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
					Ogre::TEX_TYPE_2D,
					1280,960,
					0,
					Ogre::PF_BYTE_RGBA,
					Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
			}
			Ogre::PixelBox pb(1280,960, 1, Ogre::PF_BYTE_LA, sample);
			Ogre::HardwarePixelBufferSharedPtr buffer = mTexture->getBuffer();
			buffer->blitFromMemory(pb);
		}
		return check;
	}

	void webcamstate::init(){
		this->capture_thread = boost::thread(capture_loop, this);
	}

	void webcamstate::deinit(){
		//this->capture_thread.join();
	}

	void webcamstate::capture_loop(webcamstate *self){
		//cam selection
		IMFActivate *temp = NULL;
		DeviceList list = DeviceList();
		list.EnumerateDevices();
		int count = list.Count();
		//todo: select the two C310 in right order
		temp = list.m_ppDevices[0];
		WCHAR *name=NULL;
		temp->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,&name,NULL);
		const EncodingParameters params;
		self->cap->StartCapture(temp,params);
	}

}