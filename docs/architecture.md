# RealCarMonitor Architecture

## High-Level Overview
RealCarMonitor — система мониторинга автомобиля в реальном времени.

## Components

### 1. OBD Parser (`src/obd_parser.cpp`)
- Чтение данных с CAN-шины
- Парсинг OBD-II протокола
- Предоставление: speed, rpm, engine_temp

### 2. ONNX Classifier (`src/onnx_classifier.cpp`)
- Классификация сцен через ONNX Runtime
- Вход: изображение (cv::Mat)
- Выход: класс объекта

### 3. Dashboard (`src/dashboard.cpp`)
- Отображение данных на главном экране
- OpenCV для отрисовки

### 4. DMS Monitor (`src/dms_monitor.cpp`)
- Мониторинг состояния водителя
- Анализ усталости, отвлечения

### 5. DMS HUD (`src/dms_hud.cpp`)
- HUD-интерфейс на лобовом стекле
- Предупреждения для водителя

## Data Flow