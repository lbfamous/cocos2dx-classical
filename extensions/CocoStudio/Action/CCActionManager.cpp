/****************************************************************************
 Copyright (c) 2013 cocos2d-x.org
 
 http://www.cocos2d-x.org
 
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

#include "CCActionManager.h"
#include "../Json/DictionaryHelper.h"
#include "../Json/CocoLoader.h"

NS_CC_EXT_BEGIN

static ActionManager* sharedActionManager = nullptr;

ActionManager* ActionManager::shareManager()
{
    if (!sharedActionManager) {
        sharedActionManager = new ActionManager();
    }
    return sharedActionManager;
}

void ActionManager::purge()
{
	CC_SAFE_DELETE(sharedActionManager);
}

ActionManager::ActionManager()
: m_pActionDic(nullptr)
{
	m_pActionDic = CCDictionary::create();
    CC_SAFE_RETAIN(m_pActionDic);
}

ActionManager::~ActionManager()
{
	m_pActionDic->removeAllObjects();
    CC_SAFE_RELEASE(m_pActionDic);
}

void ActionManager::initWithDictionary(const char* jsonName,const rapidjson::Value &dic,CCObject* root)
{
	std::string path = jsonName;
	int pos = path.find_last_of("/");
	std::string fileName = path.substr(pos+1,path.length());
	CCLOG("filename == %s",fileName.c_str());
	CCArray* actionList = CCArray::create();
	int actionCount = DICTOOL->getArrayCount_json(dic, "actionlist");
    for (int i=0; i< actionCount; i++) {
        ActionObject* action = new ActionObject();
		CC_SAFE_AUTORELEASE(action);
		const rapidjson::Value &actionDic = DICTOOL->getDictionaryFromArray_json(dic, "actionlist", i);
        action->initWithDictionary(actionDic,root);
        actionList->addObject(action);
    }
	m_pActionDic->setObject(actionList, fileName);
}

void ActionManager::initWithBinary(const char* file, cocos2d::CCObject *root,  CocoLoader* pCocoLoader, stExpCocoNode*	pCocoNode)
{
    std::string path = file;
	int pos = path.find_last_of("/");
	std::string fileName = path.substr(pos+1,path.length());
	CCLOG("filename == %s",fileName.c_str());
	CCArray* actionList = CCArray::create();
    
    stExpCocoNode *stChildArray = pCocoNode->GetChildArray(pCocoLoader);
    stExpCocoNode *actionNode = nullptr;
    for (int i=0; i < pCocoNode->GetChildNum(); ++i) {
        std::string key = stChildArray[i].GetName(pCocoLoader);
        if (key == "actionlist") {
            actionNode = &stChildArray[i];
            break;
        }
    }
	if (nullptr != actionNode)
    {
		int actionCount = actionNode->GetChildNum();
        for (int i = 0; i < actionCount; ++i) {
            ActionObject* action = new ActionObject();
            CC_SAFE_AUTORELEASE(action);
            
            action->initWithBinary(pCocoLoader, actionNode->GetChildArray(pCocoLoader), root);
            
            actionList->addObject(action);
        }
	}
    m_pActionDic->setObject(actionList, fileName);

}


ActionObject* ActionManager::getActionByName(const char* jsonName,const char* actionName)
{
	CCArray* actionList = (CCArray*)(m_pActionDic->objectForKey(jsonName));
	if (!actionList)
	{
		return nullptr;
	}
	for (unsigned int i=0; i<actionList->count(); i++)
	{
		ActionObject* action = dynamic_cast<ActionObject*>(actionList->objectAtIndex(i));
		if (strcmp(actionName, action->getName()) == 0)
		{
			return action;
		}
	}
	return nullptr;
}

ActionObject* ActionManager::playActionByName(const char* jsonName,const char* actionName)
{
	ActionObject* action = getActionByName(jsonName,actionName);
	if (action)
	{
		action->play();
	}
	return action;
}

ActionObject* ActionManager::playActionByName(const char* jsonName,const char* actionName, CCCallFunc* func)
{
	ActionObject* action = getActionByName(jsonName,actionName);
	if (action)
	{
		action->play(func);
	}
	return action;
}

void ActionManager::releaseActions()
{
    m_pActionDic->removeAllObjects();

}

NS_CC_EXT_END