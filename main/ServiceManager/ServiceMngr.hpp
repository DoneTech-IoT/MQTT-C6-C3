#pragma once 
#include "sdkconfig.h"
#include <memory>
#ifdef CONFIG_DONE_COMPONENT_LVGL
#include "UICoffeeMaker.hpp"
#endif
#ifdef CONFIG_DONE_COMPONENT_MATTER
#include "MatterCoffeeMaker.hpp"
#else
// #include "esp_netif.h"
// #include "protocol_examples_common.h"
#endif
#ifdef CONFIG_DONE_LOG
// #include "Custom_Log.h"
#endif
#ifdef CONFIG_DONE_COMPONENT_MQTT
#include "MQTT_CoffeeMaker.hpp"
#endif

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"

#include <esp_event.h>
#include "esp_log.h"
#include "esp_system.h"
#include <sys/param.h>

#include "ServiceBase.hpp"
#include "SharedBus.hpp"

class ServiceMngr : public ServiceBase
{
public:
    static constexpr char* mServiceName[SharedBus::ServiceID::MAX_ID] =
    {
        "",             //NO_ID
        "SRV_MNGR",     //Service Manager
        "UI",
        "MATTER",
        "MQTT",
        "LOG",      
    };    

    static constexpr uint32_t mServiceStackSize[SharedBus::ServiceID::MAX_ID] =
    {
        0 ,             //NO_ID
        30  * 1024,     //Service Manager
        100 * 1024,     //UI
        50  * 1024,     //MATTER
        30  * 1024,     //MQTT
        0               //LOG
    };
    
    explicit ServiceMngr(
        const char *TaskName,
        const SharedBus::ServiceID &ServiceID);

    ~ServiceMngr();

private:    
    static TaskHandle_t SrvMngHandle;
#ifdef CONFIG_DONE_COMPONENT_LVGL
    static TaskHandle_t LVGLHandle;
    static std::shared_ptr<UICoffeeMaker>uiCoffeeMaker;
#endif  
#ifdef CONFIG_DONE_COMPONENT_MATTER
    static TaskHandle_t MatterHandle;
    static std::shared_ptr<MatterCoffeeMaker>matterCoffeeMaker;
#endif
#ifdef CONFIG_DONE_COMPONENT_MQTT
    static TaskHandle_t MQTTHandle;
    static std::shared_ptr<MQTTCoffeeMaker>mqttCoffeeMakerApp;
#endif
    
    typedef void (*TaskKillerPtr)(void);
    typedef esp_err_t (*TaskInitPtr)(
                            TaskHandle_t *taskHandler,
                            UBaseType_t taskPriority,
                            uint32_t taskStackSize);
    typedef struct
    {                   
        SharedBus::ServiceID id;  
        TaskHandle_t taskHandler; 
        TaskInitPtr taskInit;  
        TaskKillerPtr taskKiller;      
        UBaseType_t priority;
        uint32_t taskStackSize;        
    } ServiceParams_t;

    static ServiceParams_t mServiceParams[SharedBus::ServiceID::MAX_ID];    

    /**
     * @brief Handles the transition to the machine's start state.
     * This function is called when the state machine enters the start state. 
     * It is responsible for initializing necessary components, starting tasks, 
     * or performing any setup required for this state. 
     * @return ESP_OK on successful execution.
     * @return Appropriate error code if the state transition fails.
     */
    esp_err_t OnMachineStateStart() override;             
};