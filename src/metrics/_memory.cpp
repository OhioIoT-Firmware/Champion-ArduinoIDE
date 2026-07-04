

#include "metrics/metrics.h"

#include "Arduino.h"

// int memory_check_interval = 30000;
// unsigned long memory_timer = millis();
// bool memory_first_run = true;

// TODO: to confirm, there is alreayd an interval on calls to "check all", so we don't need to gate it here


void Metrics::_check_memory(void) {

	// if ( millis() - memory_timer > memory_check_interval || memory_first_run) {	
		
		// Serial.println("\t_check_memory():");
		
		uint32_t freeHeapBytes = ESP.getFreeHeap();
		uint32_t minFreeHeapBytes = ESP.getMinFreeHeap();

		float freeHeapKB = freeHeapBytes / 1024.0;
		float minFreeHeapKB = minFreeHeapBytes / 1024.0;

		// width=0 → no minimum field width, no padding.  the natural length
		// of the formatted number is what we want anyway, and there's no
		// upper bound to worry about: even a 10 MB heap formats to 7 chars,
		// well under the 10-byte destination buffer.
		dtostrf(freeHeapKB,    0, 1, free_heap_kb);
		dtostrf(minFreeHeapKB, 0, 1, min_free_heap_kb);

		// Now you can pass these strings to MQTT
		// Serial.print("\t   - free heap (KB): ");
		// Serial.println(free_heap_kb);

		// Serial.print("\t   - min free heap (KB): ");
		// Serial.println(min_free_heap_kb);

		/*
			TODO: IF MEMORY IS LOW, _mqtt->send("warning/low memory");
		*/

		// memory_first_run = false;
		// memory_timer = millis();

	// }

}