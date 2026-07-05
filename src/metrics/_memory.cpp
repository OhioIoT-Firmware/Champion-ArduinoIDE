

#include "metrics/metrics.h"

#include "Arduino.h"



void Metrics::_check_memory(void) {

	uint32_t freeHeapBytes = ESP.getFreeHeap();
	uint32_t minFreeHeapBytes = ESP.getMinFreeHeap();

	float freeHeapKB = freeHeapBytes / 1024.0;
	float minFreeHeapKB = minFreeHeapBytes / 1024.0;

	// width=0 → no minimum field width, no padding.
	dtostrf(freeHeapKB,    0, 1, free_heap_kb);
	dtostrf(minFreeHeapKB, 0, 1, min_free_heap_kb);

	// Now you can pass these strings to MQTT
	// Serial.print("\t   - free heap (KB): ");
	// Serial.println(free_heap_kb);

	// Serial.print("\t   - min free heap (KB): ");
	// Serial.println(min_free_heap_kb);

}