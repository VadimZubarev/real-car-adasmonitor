import pandas as pd

# Читаем исходный CSV
df = pd.read_csv('data/dataset.csv')

# Оставляем только нужные столбцы
needed_columns = ['timestamp', 'RPM', 'SPEED', 'THROTTLE_POS', 'COOLANT_TEMP', 'FUEL_LEVEL']
df_filtered = df[needed_columns]

# Функция для определения стиля вождения на основе одной строки
def determine_driving_style(row):
    rpm = row['RPM']
    speed = row['SPEED']
    throttle = row['THROTTLE_POS']
    
    # Условия для определения стиля
    # AGGRESSIVE: высокие обороты, высокая скорость, резкое нажатие газа
    if rpm > 4000 or speed > 130 or throttle > 70:
        return 'AGGRESSIVE'
    
    # SLOW: низкие обороты, низкая скорость, плавное нажатие
    elif rpm < 2000 and speed < 60 and throttle < 30:
        return 'SLOW'
    
    # NORMAL: всё остальное
    else:
        return 'NORMAL'

# Альтернативная функция с более детальными условиями
def determine_driving_style_detailed(row):
    rpm = row['RPM']
    speed = row['SPEED']
    throttle = row['THROTTLE_POS']
    coolant = row['COOLANT_TEMP']
    
    # Счетчик для определения стиля
    aggressive_score = 0
    slow_score = 0
    
    # Проверка на агрессивный стиль
    if rpm > 4000:
        aggressive_score += 2
    elif rpm > 3500:
        aggressive_score += 1
    
    if speed > 120:
        aggressive_score += 2
    elif speed > 100:
        aggressive_score += 1
    
    if throttle > 70:
        aggressive_score += 2
    elif throttle > 50:
        aggressive_score += 1
    
    if coolant > 95:  # Высокая температура при агрессивной езде
        aggressive_score += 1
    
    # Проверка на медленный стиль
    if rpm < 1800:
        slow_score += 2
    elif rpm < 2200:
        slow_score += 1
    
    if speed < 50:
        slow_score += 2
    elif speed < 70:
        slow_score += 1
    
    if throttle < 20:
        slow_score += 2
    elif throttle < 30:
        slow_score += 1
    
    # Определяем стиль
    if aggressive_score >= 3:
        return 'AGGRESSIVE'
    elif slow_score >= 3:
        return 'SLOW'
    else:
        return 'NORMAL'

# Применяем функцию к каждой строке
# df_filtered['STYLE'] = df_filtered.apply(determine_driving_style, axis=1)

# Или используйте детальную версию:
df_filtered['STYLE'] = df_filtered.apply(determine_driving_style_detailed, axis=1)

# Сохраняем в новый файл
df_filtered.to_csv('data.csv', index=False)

# Выводим статистику
print(f"Было столбцов: {len(df.columns)}")
print(f"Стало столбцов: {len(df_filtered.columns)}")
print(f"Сохранено в data.csv")
print("\nРаспределение стилей вождения:")
print(df_filtered['STYLE'].value_counts())
print(f"\nПроценты:")
print(df_filtered['STYLE'].value_counts(normalize=True) * 100)