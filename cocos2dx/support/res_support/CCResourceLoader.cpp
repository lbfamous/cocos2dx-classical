/****************************************************************************
 Author: Luma (stubma@gmail.com)
 
 https://github.com/stubma/cocos2dx-better
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#include "CCResourceLoader.h"
#include "SimpleAudioEngine.h"
#include "CCArmatureDataManager.h"
#include "support/utils/CCUtils.h"
#include "CCDirector.h"

using namespace CocosDenshion;
USING_NS_CC_EXT;

NS_CC_BEGIN

static CCArray sActiveLoaders;

void ZwoptexAnimLoadTask2::load() {
    if(!CCAnimationCache::sharedAnimationCache()->animationByName(name.c_str())) {
        CCSpriteFrameCache* cache = CCSpriteFrameCache::sharedSpriteFrameCache();
        CCArray* array = CCArray::create();
        int size = frames.size();
        for(int i = 0; i < size; i++) {
            CCSpriteFrame* sf = cache->spriteFrameByName(frames.at(i).c_str());
            float& delay = durations.at(i);
            CCAnimationFrame* af = new CCAnimationFrame();
            af->initWithSpriteFrame(sf, delay, nullptr);
            CC_SAFE_AUTORELEASE(af);
            array->addObject(af);
        }
        CCAnimation* anim = CCAnimation::create(array, 1);
        anim->setRestoreOriginalFrame(restoreOriginalFrame);
        CCAnimationCache::sharedAnimationCache()->addAnimation(anim, name.c_str());
    }
}

void ZwoptexAnimLoadTask::load() {
    if(!CCAnimationCache::sharedAnimationCache()->animationByName(name.c_str())) {
        CCSpriteFrameCache* cache = CCSpriteFrameCache::sharedSpriteFrameCache();
        CCArray* array = CCArray::create();
        for(StringList::iterator iter = frames.begin(); iter != frames.end(); iter++) {
            CCSpriteFrame* f = cache->spriteFrameByName(iter->c_str());
            array->addObject(f);
        }
        CCAnimation* anim = CCAnimation::createWithSpriteFrames(array, unitDelay);
        anim->setRestoreOriginalFrame(restoreOriginalFrame);
        CCAnimationCache::sharedAnimationCache()->addAnimation(anim, name.c_str());
    }
}

void ZwoptexLoadTask::load() {
    CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile(name.c_str());
}

void EncryptedZwoptexLoadTask::load() {
    // load encryptd data
    unsigned long len;
    char* data = (char*)CCFileUtils::sharedFileUtils()->getFileData(texName.c_str(), "rb", &len);
    
    // create texture
    int decLen;
    const char* dec = nullptr;
    if(func) {
        dec = (*func)(data, len, &decLen);
    } else {
        dec = data;
        decLen = (int)len;
    }
    CCImage* image = new CCImage();
    image->initWithImageData((void*)dec, decLen);
    CC_SAFE_AUTORELEASE(image);
    CCTexture2D* tex = CCTextureCache::sharedTextureCache()->addUIImage(image, texName.c_str());
    
    // free
    if(dec != data)
        free((void*)dec);
    free(data);
    
    // add zwoptex
    CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile(name.c_str(), tex);
}

void ImageLoadTask::load() {
    CCTextureCache::sharedTextureCache()->addImage(name.c_str());
}

void CustomTask::load() {
    CCScene* scene = CCDirector::sharedDirector()->getRunningScene();
    if(scene) {
        scene->runAction(func);
    }
}

void EncryptedImageLoadTask::load() {
    // load encryptd data
    unsigned long len;
    char* data = (char*)CCFileUtils::sharedFileUtils()->getFileData(name.c_str(), "rb", &len);
    
    // create texture
    int decLen;
    const char* dec = nullptr;
    if(func) {
        dec = (*func)(data, len, &decLen);
    } else {
        dec = data;
        decLen = (int)len;
    }
    CCImage* image = new CCImage();
    image->initWithImageData((void*)dec, decLen);
    CC_SAFE_AUTORELEASE(image);
    CCTextureCache::sharedTextureCache()->addUIImage(image, name.c_str());
    
    // free
    if(dec != data)
        free((void*)dec);
    free(data);
}

void BMFontLoadTask::load() {
    CCBMFontConfiguration* conf = FNTConfigLoadFile(name.c_str());
    CCTextureCache::sharedTextureCache()->addImage(conf->getAtlasName());
}

void EncryptedBMFontLoadTask::load() {
    CCBMFontConfiguration* conf = FNTConfigLoadFile(name.c_str());
    
    // load encryptd data
    unsigned long len;
    char* data = (char*)CCFileUtils::sharedFileUtils()->getFileData(conf->getAtlasName(), "rb", &len);
    
    // create texture
    int decLen;
    const char* dec = nullptr;
    if(func) {
        dec = (*func)(data, len, &decLen);
    } else {
        dec = data;
        decLen = (int)len;
    }
    CCImage* image = new CCImage();
    image->initWithImageData((void*)dec, decLen);
    CC_SAFE_AUTORELEASE(image);
    CCTextureCache::sharedTextureCache()->addUIImage(image, conf->getAtlasName());
    
    // free
    if(dec != data)
        free((void*)dec);
    free(data);
}

void CDMusicTask::load() {
	SimpleAudioEngine::sharedEngine()->preloadBackgroundMusic(name.c_str());
}

void CDEffectTask::load() {
	SimpleAudioEngine::sharedEngine()->preloadEffect(name.c_str());
}

void ArmatureTask::load() {
    CCArmatureDataManager::sharedArmatureDataManager()->addArmatureFileInfo(configFilePath.c_str());
}

CCResourceLoader::CCResourceLoader(CCResourceLoaderListener* listener) :
m_listener(listener),
m_delay(0),
m_remainingIdle(0),
m_nextLoad(0),
m_loading(false) {
    // just add it to an array, but not hold it
    sActiveLoaders.addObject(this);
    release();
}

CCResourceLoader::~CCResourceLoader() {
    for(LoadTaskPtrList::iterator iter = m_loadTaskList.begin(); iter != m_loadTaskList.end(); iter++) {
        delete *iter;
    }
    sActiveLoaders.removeObject(this, false);
}

void CCResourceLoader::abortAll() {
    CCArray tmp;
    tmp.addObjectsFromArray(&sActiveLoaders);
    CCObject* obj;
    CCARRAY_FOREACH(&sActiveLoaders, obj) {
        CCResourceLoader* loader = (CCResourceLoader*)obj;
        loader->abort();
    }
}

void CCResourceLoader::unloadImages(const string& tex) {
    CCTextureCache::sharedTextureCache()->removeTextureForKey(tex.c_str());
}

void CCResourceLoader::unloadImages(const string& texPattern, int start, int end) {
    char buf[512];
    for(int i = start; i <= end; i++) {
        sprintf(buf, texPattern.c_str(), i);
        CCTextureCache::sharedTextureCache()->removeTextureForKey(buf);
    }
}

void CCResourceLoader::unloadSpriteFrames(const string& plistPattern, const string& texPattern, int start, int end) {
    char buf[512];
    for(int i = start; i <= end; i++) {
        sprintf(buf, plistPattern.c_str(), i);
        CCSpriteFrameCache::sharedSpriteFrameCache()->removeSpriteFramesFromFile(buf);
        sprintf(buf, texPattern.c_str(), i);
        CCTextureCache::sharedTextureCache()->removeTextureForKey(buf);
    }
}

void CCResourceLoader::unloadArmatures(string plistPattern, string texPattern, int start, int end, string config) {
    CCArmatureDataManager::sharedArmatureDataManager()->removeArmatureFileInfo(config.c_str());
    char buf[512];
    for(int i = start; i <= end; i++) {
        sprintf(buf, plistPattern.c_str(), i);
        CCSpriteFrameCache::sharedSpriteFrameCache()->removeSpriteFramesFromFile(buf);
        sprintf(buf, texPattern.c_str(), i);
        CCTextureCache::sharedTextureCache()->removeTextureForKey(buf);
    }
}

unsigned char* CCResourceLoader::loadRaw(const string& name, unsigned long* size, CC_DECRYPT_FUNC decFunc) {
    // load encryptd data
	unsigned long len;
	char* data = (char*)CCFileUtils::sharedFileUtils()->getFileData(name.c_str(), "rb", &len);
    
    // create texture
	int decLen;
    const char* dec = nullptr;
	if(decFunc) {
        dec = (*decFunc)(data, len, &decLen);
    } else {
        dec = data;
        decLen = (int)len;
    }
    
    // free
    if(dec != data)
        free(data);
    
    // save size
    if(size)
        *size = decLen;
    
    // return
    return (unsigned char*)dec;
}

string CCResourceLoader::loadString(const string& name, CC_DECRYPT_FUNC decFunc) {
    char* buf = loadCString(name, decFunc);
    string ret = buf;
    free(buf);
    return ret;
}

char* CCResourceLoader::loadCString(const string& name, CC_DECRYPT_FUNC decFunc) {
    // load encryptd data
	unsigned long len;
	char* data = (char*)CCFileUtils::sharedFileUtils()->getFileData(name.c_str(), "rb", &len);

	// create texture
	int decLen;
    const char* dec = nullptr;
	if(decFunc) {
        dec = (*decFunc)(data, len, &decLen);
    } else {
        dec = data;
        decLen = (int)len;
    }
    
    // copy as c string
    char* ret = (char*)malloc((decLen + 1) * sizeof(char));
    memcpy(ret, dec, decLen);
    ret[decLen] = 0;
    
    // free
    if(dec != data)
        free((void*)dec);
    free(data);
    
    // return
    return ret;
}

void CCResourceLoader::loadImage(const string& name, CC_DECRYPT_FUNC decFunc) {
	// load encryptd data
	unsigned long len;
	char* data = (char*)CCFileUtils::sharedFileUtils()->getFileData(name.c_str(), "rb", &len);
	
	// create texture
	int decLen;
    const char* dec = nullptr;
	if(decFunc) {
        dec = (*decFunc)(data, len, &decLen);
    } else {
        dec = data;
        decLen = (int)len;
    }
	CCImage* image = new CCImage();
	image->initWithImageData((void*)dec, decLen);
	CC_SAFE_AUTORELEASE(image);
	CCTextureCache::sharedTextureCache()->addUIImage(image, name.c_str());
	
	// free
    if(dec != data)
        free((void*)dec);
	free(data);
}

void CCResourceLoader::loadZwoptex(const string& plistName, const string& texName, CC_DECRYPT_FUNC decFunc) {
	// load encryptd data
	unsigned long len;
	char* data = (char*)CCFileUtils::sharedFileUtils()->getFileData(texName.c_str(), "rb", &len);
	
	// create texture
	int decLen;
	const char* dec = nullptr;
	if(decFunc) {
        dec = (*decFunc)(data, len, &decLen);
    } else {
        dec = data;
        decLen = (int)len;
    }
    CCImage* image = new CCImage();
	image->initWithImageData((void*)dec, decLen);
	CC_SAFE_AUTORELEASE(image);
	CCTexture2D* tex = CCTextureCache::sharedTextureCache()->addUIImage(image, texName.c_str());
	
	// free
    if(dec != data)
        free((void*)dec);
	free(data);
	
	// add zwoptex
	CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile(plistName.c_str(), tex);
}

void CCResourceLoader::run() {
    if(m_loading)
        return;
    m_loading = true;
    
	CCScheduler* scheduler = CCDirector::sharedDirector()->getScheduler();
	scheduler->scheduleSelector(schedule_selector(CCResourceLoader::doLoad), this, 0, kCCRepeatForever, m_delay, false);
}

void CCResourceLoader::runInBlockMode() {
    m_loading = true;
    for(LoadTaskPtrList::iterator iter = m_loadTaskList.begin(); iter != m_loadTaskList.end(); iter++) {
        CCResourceLoadTask* lp = *iter;
        lp->load();
        if(m_listener)
            m_listener->onResourceLoadingProgress(m_nextLoad * 100 / m_loadTaskList.size(), 0);
    }
    m_loading = false;
}

void CCResourceLoader::abort() {
    if(!m_loading)
        return;
    m_loading = false;
    
    CCScheduler* scheduler = CCDirector::sharedDirector()->getScheduler();
    scheduler->unscheduleSelector(schedule_selector(CCResourceLoader::doLoad), this);
    autorelease();
}

void CCResourceLoader::addAndroidStringTask(const string& lan, const string& path, bool merge) {
    AndroidStringLoadTask* t = new AndroidStringLoadTask();
    t->lan = lan;
    t->path = path;
    t->merge = merge;
    addLoadTask(t);
}

void CCResourceLoader::addImageTask(const string& name) {
	ImageLoadTask* t = new ImageLoadTask();
    t->name = name;
    addLoadTask(t);
}

void CCResourceLoader::addImageTask(const string& name, CC_DECRYPT_FUNC decFunc) {
	EncryptedImageLoadTask* t = new EncryptedImageLoadTask();
	t->name = name;
	t->func = decFunc;
	addLoadTask(t);
}

void CCResourceLoader::addBMFontTask(const string& fntFile) {
    BMFontLoadTask* t = new BMFontLoadTask();
    t->name = fntFile;
    addLoadTask(t);
}

void CCResourceLoader::addBMFontTask(const string& fntFile, CC_DECRYPT_FUNC decFunc) {
    EncryptedBMFontLoadTask* t = new EncryptedBMFontLoadTask();
    t->name = fntFile;
    t->func = decFunc;
    addLoadTask(t);
}

void CCResourceLoader::addAtlasTaskByPlist(const string& name) {
    ZwoptexLoadTask* t = new ZwoptexLoadTask();
    t->name = name;
    addLoadTask(t);
}

void CCResourceLoader::addAtlasTaskByPlistPattern(const string& pattern, int start, int end) {
	char buf[512];
	for(int i = start; i <= end; i++) {
		sprintf(buf, pattern.c_str(), i);
		addAtlasTaskByPlist(buf);
	}
}

void CCResourceLoader::addAtlasTaskByPlistAndImage(const string& plistName, const string& texName) {
    addAtlasTaskByPlistAndImage(plistName, texName, nullptr);
}

void CCResourceLoader::addAtlasTaskByPlistAndImage(const string& plistName, const string& texName, CC_DECRYPT_FUNC decFunc) {
	EncryptedZwoptexLoadTask* t = new EncryptedZwoptexLoadTask();
	t->name = plistName;
	t->texName = texName;
	t->func = decFunc;
	addLoadTask(t);
}

void CCResourceLoader::addAtlasTaskByPlistAndImagePattern(const string& plistPattern, const string& texPattern, int start, int end) {
    addAtlasTaskByPlistAndImagePattern(plistPattern, texPattern, start, end, nullptr);
}

void CCResourceLoader::addAtlasTaskByPlistAndImagePattern(const string& plistPattern, const string& texPattern, int start, int end, CC_DECRYPT_FUNC decFunc) {
	char buf1[512], buf2[512];
	for(int i = start; i <= end; i++) {
		sprintf(buf1, plistPattern.c_str(), i);
		sprintf(buf2, texPattern.c_str(), i);
		addAtlasTaskByPlistAndImage(buf1, buf2, decFunc);
	}
}

void CCResourceLoader::addAtlasAnimByFramePattern(const string& name,
                                                  float unitDelay,
                                                  const string& pattern,
                                                  int startIndex,
                                                  int endIndex,
                                                  bool restoreOriginalFrame) {
	ZwoptexAnimLoadTask* t = new ZwoptexAnimLoadTask();
	t->name = name;
	t->unitDelay = unitDelay;
	t->restoreOriginalFrame = restoreOriginalFrame;
	char buf[256];
	for(int i = startIndex; i <= endIndex; i++) {
		sprintf(buf, pattern.c_str(), i);
		t->frames.push_back(buf);
	}
	addLoadTask(t);
}

void CCResourceLoader::addAtlasAnimByFramePatternAndVariableDelay(const string& name,
                                                                  const string& pattern,
                                                                  int startIndex,
                                                                  int endIndex,
                                                                  const string& delayString,
                                                                  bool restoreOriginalFrame) {
    ZwoptexAnimLoadTask2* t = new ZwoptexAnimLoadTask2();
    t->name = name;
    t->restoreOriginalFrame = restoreOriginalFrame;
    
    char buf[256];
    for(int i = startIndex; i <= endIndex; i++) {
        sprintf(buf, pattern.c_str(), i);
        t->frames.push_back(buf);
    }
    
    CCObject* obj;
    const CCArray& delays = CCUtils::arrayFromString(delayString);
    CCARRAY_FOREACH(&delays, obj) {
        CCFloat* f = (CCFloat*)obj;
        t->durations.push_back(f->getValue());
    }
    
    addLoadTask(t);
}

void CCResourceLoader::addAtlasAnimByFramePatternAndVariableIndex(const string& name,
                                                                  const string& pattern,
                                                                  const string& indicesString,
                                                                  float delay,
                                                                  bool restoreOriginalFrame) {
    // task
    ZwoptexAnimLoadTask2* t = new ZwoptexAnimLoadTask2();
    t->name = name;
    t->restoreOriginalFrame = restoreOriginalFrame;
    
    // frame names
    char buf[256];
    const CCArray& indices = CCUtils::arrayFromString(indicesString);
    CCObject* obj;
    CCARRAY_FOREACH(&indices, obj) {
        sprintf(buf, pattern.c_str(), (int)((CCFloat*)obj)->getValue());
        t->frames.push_back(buf);
    }
    
    // delay
    for(int i = 0; i < indices.count(); i++) {
        t->durations.push_back(delay);
    }
    
    addLoadTask(t);
}

void CCResourceLoader::addAtlasAnimByFramePatternAndVariableIndexDelay(const string& name,
                                                                       const string& pattern,
                                                                       const string& indicesString,
                                                                       const string& delayString,
                                                                       bool restoreOriginalFrame) {
    // task
    ZwoptexAnimLoadTask2* t = new ZwoptexAnimLoadTask2();
    t->name = name;
    t->restoreOriginalFrame = restoreOriginalFrame;
    
    // frame names
    char buf[256];
    const CCArray& indices = CCUtils::arrayFromString(indicesString);
    CCObject* obj;
    CCARRAY_FOREACH(&indices, obj) {
        sprintf(buf, pattern.c_str(), (int)((CCFloat*)obj)->getValue());
        t->frames.push_back(buf);
    }
    
    // delays
    const CCArray& delays = CCUtils::arrayFromString(delayString);
    CCARRAY_FOREACH(&delays, obj) {
        CCFloat* f = (CCFloat*)obj;
        t->durations.push_back(f->getValue());
    }
    
    addLoadTask(t);
}

void CCResourceLoader::addCDEffectTask(const string& name) {
	CDEffectTask* t = new CDEffectTask();
	t->name = name;
	addLoadTask(t);
}

void CCResourceLoader::addCDMusicTask(const string& name) {
	CDMusicTask* t = new CDMusicTask();
	t->name = name;
	addLoadTask(t);
}

void CCResourceLoader::addCustomTask(CCCallFunc* func) {
    CustomTask* t = new CustomTask();
    t->func = func;
    CC_SAFE_RETAIN(t->func);
    addLoadTask(t);
}

void CCResourceLoader::addArmatureTask(string config) {
    ArmatureTask* t = new ArmatureTask();
    t->configFilePath = config;
    addLoadTask(t);
}

void CCResourceLoader::addArmatureTask(string plist, string tex, string config) {
    addArmatureTask(plist, tex, config, nullptr);
}

void CCResourceLoader::addArmatureTask(string plist, string tex, string config, CC_DECRYPT_FUNC func) {
    if(!plist.empty() && !tex.empty()) {
        addAtlasTaskByPlistAndImage(plist, tex, func);
    }
    
    if(!config.empty())
        addArmatureTask(config);
}

void CCResourceLoader::addArmatureTask(string plistPattern, string texPattern, int start, int end, string config) {
    addArmatureTask(plistPattern, texPattern, start, end, config, nullptr);
}

void CCResourceLoader::addArmatureTask(string plistPattern, string texPattern, int start, int end, string config, CC_DECRYPT_FUNC func) {
    char buf1[512], buf2[512];
	for(int i = start; i <= end; i++) {
		sprintf(buf1, plistPattern.c_str(), i);
		sprintf(buf2, texPattern.c_str(), i);
		addAtlasTaskByPlistAndImage(buf1, buf2, func);
	}
    
    if(!config.empty())
        addArmatureTask(config);
}

void CCResourceLoader::addLoadTask(CCResourceLoadTask* t) {
    m_loadTaskList.push_back(t);
}

void CCResourceLoader::doLoad(float delta) {
    if(m_remainingIdle > 0) {
        m_remainingIdle -= delta;
    } else if(m_loadTaskList.size() <= m_nextLoad) {
        if(m_loading) {
            m_loading = false;
            CCScheduler* scheduler = CCDirector::sharedDirector()->getScheduler();
            scheduler->unscheduleSelector(schedule_selector(CCResourceLoader::doLoad), this);
            autorelease();
        }
        
        if(m_listener)
            m_listener->onResourceLoadingDone();
    } else {
        CCResourceLoadTask* lp = m_loadTaskList.at(m_nextLoad++);
        m_remainingIdle = 0;
        
        lp->load();
        if(m_listener)
            m_listener->onResourceLoadingProgress(m_nextLoad * 100 / m_loadTaskList.size(), delta);
    }
}

NS_CC_END