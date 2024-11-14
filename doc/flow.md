```mermaid
graph TD
    A[Start] --> B[Setup]
    B --> C[Initialize BLE Service]
    C --> D[Create Tasks]

    D --> E[BLE Task]
    D --> F[Main Task]

    E --> G{Client Connected?}
    G -->|Yes| H[Handle BLE Operations]
    G -->|No| I[Advertise]
    H --> J[Send Keep-Alive]
    J --> G

    F --> K{Check Command}
    K -->|Preview| L[Preview Flow]
    K -->|Capture| M[Capture Flow]
    K -->|Inference| N[Inference Flow]

    L --> O[Init Camera Preview]
    O --> P[Start WiFi AP]
    P --> Q[Start MJPEG Server]
    Q --> R{Stream Active?}
    R -->|Yes| S[Stream Frame]
    S --> T[Update Metrics]
    T --> R
    R -->|No| K

    M --> U[Init Camera Capture]
    U --> V[Init SD Card]
    V --> W{Capture Timer?}
    W -->|Yes| X[Capture Image]
    X --> Y[Save RGB]
    Y --> Z[Convert & Save JPEG]
    Z --> AA[Update Metrics]
    AA --> W
    W -->|No| K

    N --> AB[Init Model]
    AB --> AC[Process Frame]
    AC --> AD[Update Results]
    AD --> K
```
