## Fault Code Table
Code | Source | Description | Trigger Condition
--- | --- | --- | ---
F00 | Manual | Test fault | Triggered via button push
F01 | HC-SR04 | Object jam | Distance < 10 cm for more than 2 seconds
F02 | DHT22 | High temperature | Temperature > 30°C
F03 | Photoresistor (LDR) | Blocked light sensor | Light level < 200 (ADC, 0-1023)

## State Diagram
---
config:
  theme: redux
  layout: fixed
---
flowchart TB
    n2["IDLE"] <-- button press --> n3["RUNNING"]
    n5["FAULT"] -- button press --> n4["RESET_REQUIRED"]
    n4 -- button press --> n2
    n3 -- F00 / F01 / F02 / F03 --> n5
    n6["Junction"] -- "startup (self-test failed)" --> n2
    n6 L_n6_n3_0@-- "startup (self-test passed)" --> n3

    n2@{ shape: event}
    n3@{ shape: event}
    n5@{ shape: event}
    n4@{ shape: event}
    n6@{ shape: junction}

    L_n6_n3_0@{ animation: none }
