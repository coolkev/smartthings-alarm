/**
 *  Arduino Alarm Controller
 *
 *  Copyright 2014 Kevin Lewis
 *
 */
definition(
    name: "Arduino Alarm Controller",
    namespace: "coolkev",
    author: "Kevin Lewis",
    description: "Turn your hardwired alarm into smart sensors",
    category: "Safety & Security",
    iconUrl: "https://s3.amazonaws.com/smartapp-icons/Convenience/Cat-Convenience.png",
    iconX2Url: "https://s3.amazonaws.com/smartapp-icons/Convenience/Cat-Convenience@2x.png")

 
preferences {
	

	section("Zones") {
    	for (int i=1;i<=8;i++) {
			input "zone" + i, title: "Zone " + i, "string", description:"Zone " + i, required: false
            input "typezone" + i, "enum", title: "Type", options: ["Open/Closed Sensor","Motion Detector"], required: false
        }
        
	}
    section("Which Arduino shield?") {
		input "arduino", title: "Shield","device.arduinoThingShield"
    }    
    
    
}

def installed() {
	log.debug "Installed with settings: ${settings}"
	initialize()
}

def updated() {
	log.debug "Updated with settings: ${settings}"
	unsubscribe()
	initialize()
}


def initialize() {

    
    // Listen to anything which happens on the device
    subscribe(arduino, "response", zonestatusChanged)
    
    
    settings.each {s ->
    	
    	def name = s.getKey()
		def value = s.getValue()
        
		if (name.startsWith("zone")) {

            log.debug "checking device: ${name}, value: $value"
			
            def zoneType = settings["type" + name];
			
            if (zoneType == null || zoneType == "")
            {
            	zoneType = "Open/Closed Sensor"
            }
            
            def existingDevice = getChildDevice(name)
            if(!existingDevice) {
                log.debug "creating device: ${name}"
            	def childDevice = addChildDevice("smartthings", zoneType, name, null, [name: "Device.${name}", label: value, completedSetup: true])
            }
            else {
            	//log.debug existingDevice.deviceType
                //existingDevice.type = zoneType
                existingDevice.label = value
                existingDevice.take()
                log.debug "device already exists: ${name}"

            }

        }
    }
    
    
    def delete = getChildDevices().findAll { it.deviceNetworkId.startsWith("zone") && !settings[it.deviceNetworkId] }

    delete.each {
        log.debug "deleting child device: ${it.deviceNetworkId}"
        deleteChildDevice(it.deviceNetworkId)
    }
}

def uninstalled() {
    removeChildDevices(getChildDevices())
}

private removeChildDevices(delete) {
    delete.each {
    	log.debug "deleting child device: ${it.deviceNetworkId}"
        deleteChildDevice(it.deviceNetworkId)
    }
}
def zonestatusChanged(evt)
{
	
    
    log.debug "zonestatusChanged ${evt.value}"
    
    
    def parts = evt.value.split();
    
    def zone = parts[1]
    def status = parts[2]
    
    log.debug "zone ${zone}"
    log.debug "status ${status}"
    
    def device = getChildDevice("zone${zone}")
    
    if (device)
    {
    	log.debug "$device statusChanged $status"
        
        def zoneType = settings["typezone" + zone];
			
        if (zoneType == null || zoneType == "")
        {
            zoneType = "Open/Closed Sensor"
        }

		def eventName = "contact"
        
        if (zoneType=="Motion Detector")
        {
        	eventName = "motion";
            status = status=="open" ? "active" : "inactive"
        }   
        
        device.sendEvent(name: eventName, value: status, isStateChange:true)
    }
    else {
    
    	log.debug "couldn't find device for zone ${zone}"
    	
    }
}
