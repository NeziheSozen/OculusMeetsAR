#include "ARLib/Webcam/webcamstate.h"

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif
#include <stdint.h>

#include <OGRE/OgreTextureManager.h>
#include <OGRE/OgreHardwarePixelBuffer.h>
#include <OGRE/OgreResourceGroupManager.h>
#include <OGRE/OgreStringConverter.h>

namespace webcam{
	webcamstate::webcamstate(int camNum)
	{
		camNumber=camNum;
		CCapture::CreateInstance(&cap);
	}


	webcamstate::~webcamstate()
	{
	}

	HRESULT webcamstate::update()
	{
		HRESULT check=E_FAIL;
		BYTE* sample = cap->getLastImagesample(&check);
		if(SUCCEEDED(check)){
			if (mTexture.isNull())
			{
				static int i = 0;
				mTexture = Ogre::TextureManager::getSingleton().createManual(
					"ARLibWebcam/VideoTexture" + Ogre::StringConverter::toString(++i),
					Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
					Ogre::TEX_TYPE_2D,
					1280,960,
					0,
					Ogre::PF_BYTE_BGR,
					Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
			}
			Ogre::PixelBox pb(1280,960, 1, Ogre::PF_BYTE_BGR, sample);
			Ogre::HardwarePixelBufferSharedPtr buffer = mTexture->getBuffer();
			buffer->blitFromMemory(pb);
		}
		return check;
	}

	void webcamstate::deinit(){
	}

	void webcamstate::init(){
		//cam selection
		IMFActivate *temp = NULL;
		DeviceList list = DeviceList();
		list.EnumerateDevices();
		int count = list.Count();

		//todo: select the two C310 in right order
		bool firstfound=false;
		for(unsigned int i=0;i<list.Count();i++){
			temp=list.m_ppDevices[i];
			WCHAR *name=NULL;
			temp->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,&name,NULL);
			std::string displayname((char*) name);
			

			if(displayname.compare(/*"USB-Videoger�t"*/ "Logitech HD Webcam C310")){
				if(camNumber==0){
					break;
				}
				else if(!firstfound){
					firstfound=true;
				}
				else if(firstfound){
					break;
				}
			}

		}
		//const EncodingParameters params = EncodingParameters();
		cap->StartCapture(temp);
	}

}