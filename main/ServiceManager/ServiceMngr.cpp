#include "nvsFlash.h"
#include <cstring>
#include "esp_heap_caps.h"

#include "ServiceMngr.hpp"
#include "Singleton.hpp"
#include "SpiffsManager.h"

static const char* TAG = "ServiceMngr";
TaskHandle_t ServiceMngr::SrvMngHandle = nullptr;
ServiceMngr::ServiceParams_t ServiceMngr::mServiceParams[SharedBus::ServiceID::MAX_ID];
#ifdef CONFIG_DONE_COMPONENT_LVGL
TaskHandle_t ServiceMngr::LVGLHandle = nullptr;
std::shared_ptr<UICoffeeMaker> ServiceMngr::uiCoffeeMaker;
#endif  
#ifdef CONFIG_DONE_COMPONENT_MATTER
TaskHandle_t ServiceMngr::MatterHandle = nullptr;
std::shared_ptr<MatterCoffeeMaker> ServiceMngr::matterCoffeeMaker;
#endif
#ifdef CONFIG_DONE_COMPONENT_MQTT
TaskHandle_t ServiceMngr::MQTTHandle = nullptr;
std::shared_ptr<MQTTCoffeeMaker> ServiceMngr::mqttCoffeeMakerApp;
#endif

#include "esp_heap_caps.h"

ServiceMngr::ServiceMngr(
    const char *TaskName,
    const SharedBus::ServiceID &ServiceID) :
    ServiceBase(TaskName, ServiceID)
{
    esp_err_t err;    

    nvsFlashInit();

    SpiffsInit();

    SharedBus sharedBus;
    if(sharedBus.Init() == ESP_OK)
    {
        ESP_LOGI(TAG, "Initialized SharedBus successfully");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to Initialize SharedBus.");
    }                

#ifdef MONITORING
// char pcTaskList[TASK_LIST_BUFFER_SIZE];
#endif    

    err = TaskInit(
            &SrvMngHandle,
            tskIDLE_PRIORITY + 1,
            mServiceStackSize[SharedBus::ServiceID::SERVICE_MANAGER]);
    
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG,"%s service created.",
            mServiceName[SharedBus::ServiceID::SERVICE_MANAGER]);
    }     
    else 
    {
        ESP_LOGE(TAG,"failed to create %s service.",
            mServiceName[SharedBus::ServiceID::SERVICE_MANAGER]);
    }        
}

ServiceMngr::~ServiceMngr()
{                
}

esp_err_t ServiceMngr::OnMachineStateStart()
{
    esp_err_t err = ESP_OK;
    
#ifdef CONFIG_DONE_COMPONENT_LVGL
    uiCoffeeMaker = Singleton<UICoffeeMaker, const char*, SharedBus::ServiceID>::
                        GetInstance(static_cast<const char*>
                        (mServiceName[SharedBus::ServiceID::UI]),
                        SharedBus::ServiceID::UI);                
            
    err = uiCoffeeMaker->TaskInit(
        &LVGLHandle,
        tskIDLE_PRIORITY + 1,
        mServiceStackSize[SharedBus::ServiceID::UI]);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG,"%s service created.",
            mServiceName[SharedBus::ServiceID::UI]);
    }
    else
    {
        ESP_LOGE(TAG,"failed to create %s service",
            mServiceName[SharedBus::ServiceID::UI]);
    }    
#endif //CONFIG_DONE_COMPONENT_LVGL

#ifdef CONFIG_DONE_COMPONENT_MATTER 
    matterCoffeeMaker = Singleton<MatterCoffeeMaker, const char*, SharedBus::ServiceID>::
                            GetInstance(static_cast<const char*>
                            (mServiceName[SharedBus::ServiceID::MATTER]), 
                            SharedBus::ServiceID::MATTER);    
    
    err = matterCoffeeMaker->TaskInit(
            &MatterHandle,
            tskIDLE_PRIORITY + 1,
            mServiceStackSize[SharedBus::ServiceID::MATTER]);
    
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG,"%s service created",
            mServiceName[SharedBus::ServiceID::MATTER]);
    }     
    else
    {
        ESP_LOGE(TAG, "failed to create %s service",
                 mServiceName[SharedBus::ServiceID::MATTER]);
    }
#endif // CONFIG_DONE_COMPONENT_MATTER

#ifdef CONFIG_DONE_COMPONENT_MQTT
    mqttCoffeeMakerApp = Singleton<MQTTCoffeeMaker, const char *, SharedBus::ServiceID>::
        GetInstance(static_cast<const char *>(mServiceName[SharedBus::ServiceID::MQTT]),
                    SharedBus::ServiceID::MQTT);

    err = mqttCoffeeMakerApp->TaskInit(
        &MQTTHandle,
        tskIDLE_PRIORITY + 1,
        mServiceStackSize[SharedBus::ServiceID::MQTT]);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "%s service created.",
                 mServiceName[SharedBus::ServiceID::MQTT]);
    }
    else
    {
        ESP_LOGE(TAG, "failed to create %s service.",
                 mServiceName[SharedBus::ServiceID::MQTT]);
    }
#endif // CONFIG_DONE_COMPONENT_MQTT

    return err;
}