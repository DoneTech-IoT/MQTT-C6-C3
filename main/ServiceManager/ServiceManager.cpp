#include "ServiceManager.h"
#include "SharedBus.h"
#include "nvsFlash.h"

#ifdef CONFIG_DONE_COMPONENT_LVGL
#include "GUI.h"
#include "coffeeMaker_GUI.h"
#endif
#ifdef CONFIG_DONE_COMPONENT_MATTER
#include "MatterInterface.h"
#else
#include "esp_netif.h"
#include "protocol_examples_common.h"
#endif
#ifdef CONFIG_DONE_LOG
#include "Custom_Log.h"
#endif
#ifdef CONFIG_DONE_COMPONENT_MQTT
#include "MQTT_Interface.h"
#endif

// ****************************** Local Variables
typedef enum 
{
    IDLE = 0,
    INIT,
    START,
    ACTIVE, 
}DaemonState;
static DaemonState State;

static const char *TAG = "Service_Manager";

#ifdef CONFIG_DONE_COMPONENT_LVGL
static TaskHandle_t LVGLHandle = NULL;
#endif  //CONFIG_DONE_COMPONENT_LVGL

#ifdef CONFIG_DONE_COMPONENT_MATTER
static TaskHandle_t MatterHandle = NULL;
#endif  //CONFIG_DONE_COMPONENT_MATTER

#ifdef CONFIG_DONE_COMPONENT_MQTT
static TaskHandle_t MQTTHandle = NULL;
#endif  //CONFIG_DONE_COMPONENT_MQTT

extern SharedBusPacket_t SharedBusPacket;
static MatterEventPacket *MatterEventReceivedPacket;

#define TASK_LIST_BUFFER_SIZE 512
// #define MONITORING

// Global instance of Service Manager
static ServiceManger_t ServiceManger;

/**
 * @brief Deletes a task.
 * This function deletes the specified task.
 * @param TaskNumber The number of the task to be deleted.
 * @return void
 */
void TaskKiller(int TaskNumber)
{
    ServiceManger.Services[TaskNumber].TaskKiller(&ServiceManger.Services[TaskNumber].taskHandler);
    ESP_LOGI(TAG, "Task %d Deleted !", TaskNumber);
}

/* 
    * @brief run given service.
    * This function runs the given service by initializing the service parameters and creating the task.
    * @param[in] serviceParams Service parameters
    * @param[in] id Service to run
    * @retval ESP_OK if the service is run successfully, otherwise ESP_FAIL
*/ 
esp_err_t ServiceManager_RunService(ServiceParams_t serviceParams)
{
    esp_err_t err = ESP_OK;

    // Call the function pointer (init_func) with proper arguments
    TaskInitPtr init_func = serviceParams.TaskInit;
    err = init_func(&serviceParams.taskHandler, 
                    serviceParams.priority, 
                    serviceParams.taskStack);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "Failed to create MQTT!");
        return err;
    }

    return ESP_OK;
}

static void ServiceManger_RunAllDaemons()
{    
    esp_err_t err;

#ifdef CONFIG_DONE_COMPONENT_LVGL    
    ServiceParams_t GUIParams;
    GUIParams.maximumRAM_Needed = LVGL_STACK * 2;
    strcpy(GUIParams.name, "GUI");
    GUIParams.ramType = PSRAM_;
    GUIParams.TaskKiller = GUI_TaskKill;
    GUIParams.taskStack = LVGL_STACK;
    GUIParams.priority = tskIDLE_PRIORITY + 1;  
    GUIParams.taskHandler = LVGLHandle;  
    GUIParams.TaskInit = GUI_TaskInit;
    GUI_OnInitElements(GUI_Setup);
    GUI_OnSharedBusReceived(GUI_ProcessSharedBusMsg);
    err = ServiceManager_RunService(GUIParams);
    if (err)
    {
        ESP_LOGE(TAG, "Failed to create GUI!");
    }
    else
    {
        ESP_LOGI(TAG, "GUI Daemon Created !");        
    }    
#endif //CONFIG_DONE_COMPONENT_LVGL

#ifdef CONFIG_DONE_COMPONENT_MATTER    
    ServiceParams_t MatterParams;
    strcpy(MatterParams.name, "Matter");    
    MatterParams.maximumRAM_Needed = 0;
    MatterParams.ramType = SRAM_;
    MatterParams.TaskKiller = Matter_TaskKill;
    MatterParams.taskStack = MATTER_STACK_SIZE;
    MatterParams.priority = tskIDLE_PRIORITY + 1;
    MatterParams.taskHandler = MatterHandle;
    MatterParams.TaskInit = Matter_TaskInit;
    err = ServiceManager_RunService(MatterParams);
    if (err)
    {
        ESP_LOGE(TAG, "Failed to create Matter !");
    }
    else 
    {
        ESP_LOGI(TAG, "Matter Daemon Created !");
    }
#endif //CONFIG_DONE_COMPONENT_MATTER
    
#ifdef CONFIG_DONE_COMPONENT_MQTT    
    ServiceParams_t MQTTParams;
    strcpy(MQTTParams.name, "MQTT");
    MQTTParams.maximumRAM_Needed = 0;                
    MQTTParams.ramType = SRAM_;
    MQTTParams.TaskKiller = MQTT_TaskKill;
    MQTTParams.taskStack = MQTT_STACK;
    MQTTParams.priority = tskIDLE_PRIORITY + 1;
    MQTTParams.taskHandler = MQTTHandle;
    MQTTParams.TaskInit = MQTT_TaskInit;
    err = ServiceManager_RunService (MQTTParams);
    if (err)
    {
        ESP_LOGE(TAG, "Failed to create MQTT !");
    }    
    else 
    {
        ESP_LOGI(TAG, "MQTT Daemon Created !");
    }
#endif //CONFIG_DONE_COMPONENT_MQTT
}

/**
 * @brief Task function for the Service Manager task.
 * This task initializes and manages other tasks.
 * @param pvParameter Pointer to task parameters.
 * @return void
 */
static void ServiceManger_MainTask(void *pvParameter)
{    
    nvsFlashInit();
    
    if (SharedBus_Init())
    {
        ESP_LOGI(TAG, "initialized SharedBus successfully");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to Initialize SharedBus.");
    }                

#ifdef MONITORING
// char pcTaskList[TASK_LIST_BUFFER_SIZE];
#endif                
    
    State = DaemonState::INIT;
    while (true)
    {        
        if(SharedBus_Receive(&SharedBusPacket, SERVICE_MANAGER_SERVICE_ID))        
        {                             
            
        }
                
        switch (State)
        {
            case DaemonState::IDLE:
                break;
            case DaemonState::INIT:   
                SharedBus_DaemonRegistered(SERVICE_MANAGER_SERVICE_ID);
                State = DaemonState::START;
                break;

            case DaemonState::START:        
                ESP_LOGI(TAG, "Service Manager Daemon Created !");            
                ServiceManger_RunAllDaemons();              
                SharedBus_NotifyTaskExecution();
                State = DaemonState::ACTIVE;
                break;

            case DaemonState::ACTIVE:
                //TODO: call ProcessMessageFunction to 
                //TODO: process sharedbus messages
                break;

            default:
                break;
        }                                                     

        vTaskDelay(pdMS_TO_TICKS(1));

#ifdef MONITORING
// vTaskList(pcTaskList);
// ESP_LOGI(TAG, "Task List:\n%s\n", pcTaskList);
#endif                
    }
}

/**
 * @brief Initializes the Service Manager task.
 * This function initializes the Service Manager task by allocating memory and creating the task.
 * @return void
 */
void ServiceManger_TaskInit()
{
    StaticTask_t *xTaskServiceMangerBuffer = (StaticTask_t *)malloc(sizeof(StaticTask_t));
    StackType_t *xServiceMangerStack = (StackType_t *)malloc(SERVICE_MANGER_STACK * sizeof(StackType_t));
    xTaskCreateStatic(
        ServiceManger_MainTask,         // Task function
        "ServiceMangerTask",       // Task name (for debugging)
        SERVICE_MANGER_STACK,      // Stack size (in words)
        NULL,                      // Task parameters (passed to the task function)
        tskIDLE_PRIORITY + 1,      // Task priority (adjust as needed)
        xServiceMangerStack,       // Stack buffer
        xTaskServiceMangerBuffer); // Task control block
}