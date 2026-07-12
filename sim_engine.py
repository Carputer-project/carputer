#!/usr/bin/env python3
"""
Engine simulator for carputer.
Sends fake sensor data over UDP to 192.168.1.100:5001
so the dashboard gauges work without the ESP32.

Usage: python3 sim_engine.py [target_ip] [interval_ms]
  Default: python3 sim_engine.py 192.168.1.100 200
"""

import socket
import json
import math
import time
import sys
import random

TARGET = sys.argv[1] if len(sys.argv) > 1 else "192.168.1.100"
PORT = 5001
INTERVAL = int(sys.argv[2]) if len(sys.argv) > 2 else 200  # ms

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Engine state
rpm = 800
throttle = 0
speed = 0
coolant = 20       # cold start
oil = 20
o2afr = 14.7
fuel = 75
gear = 1
running = True

# Gear ratios (3S-FE / S54)
GEAR_RATIOS = [3.83, 2.05, 1.33, 0.97, 0.73]
FINAL_DRIVE = 4.176
TIRE_CIRCUMFERENCE = 1.936  # meters (205/45R17)

tick = 0

print(f"Simulating engine -> {TARGET}:{PORT} (Ctrl+C to stop)")

while running:
    try:
        t = tick * (INTERVAL / 1000.0)

        # --- Throttle: gentle drive cycle ---
        # idle → accelerate → cruise → decelerate → idle (60s cycle)
        cycle = t % 60
        if cycle < 5:
            throttle = 0
        elif cycle < 12:
            throttle = min(80, (cycle - 5) * 12)
        elif cycle < 25:
            throttle = 80 + 10 * math.sin(t * 0.5)
        elif cycle < 32:
            throttle = max(0, 80 - (cycle - 25) * 12)
        else:
            throttle = 0

        throttle += random.uniform(-2, 2)
        throttle = max(0, min(100, throttle))

        # --- RPM from throttle + gear ---
        idle_rpm = 800
        max_rpm = 6500
        target_rpm = idle_rpm + (max_rpm - idle_rpm) * (throttle / 100.0)

        # Gear selection based on RPM
        if throttle < 5:
            gear = 0  # neutral/idle
        else:
            for i, gr in enumerate(GEAR_RATIOS):
                total_ratio = gr * FINAL_DRIVE
                speed_from_rpm = (target_rpm * TIRE_CIRCUMFERENCE * 60) / (total_ratio * 1000)
                if speed_from_rpm < 160:
                    gear = i + 1
                    break
            else:
                gear = 5

        # RPM follows throttle with some lag
        rpm += (target_rpm - rpm) * 0.15
        rpm += random.uniform(-30, 30)
        rpm = max(700, min(7000, rpm))

        # --- Speed from RPM in gear ---
        if gear >= 1:
            total_ratio = GEAR_RATIOS[gear - 1] * FINAL_DRIVE
            speed = (rpm * TIRE_CIRCUMFERENCE * 60) / (total_ratio * 1000)
        else:
            speed *= 0.95  # coast to stop
            if speed < 1:
                speed = 0

        speed += random.uniform(-1, 1)
        speed = max(0, min(200, speed))

        # --- Temperatures ---
        # Coolant warms up to ~90C over 5 minutes, then fluctuates
        if coolant < 88:
            coolant += 0.05
        elif coolant > 92:
            coolant -= 0.03
        coolant += random.uniform(-0.3, 0.3)
        coolant = max(20, min(115, coolant))

        # Oil follows coolant but hotter
        oil = coolant + 15 + random.uniform(-1, 1)
        oil = max(20, min(140, oil))

        # Intake = ambient + heat soak
        intake = 35 + (rpm / 7000) * 15 + random.uniform(-1, 1)

        # --- O2 AFR ---
        if throttle > 60:
            o2afr = 12.5 + random.uniform(-0.5, 0.5)  # rich under load
        elif throttle < 10:
            o2afr = 14.7 + random.uniform(-0.3, 0.3)  # stoich idle
        else:
            o2afr = 13.8 + random.uniform(-0.5, 0.5)  # cruise
        o2afr = max(10.0, min(17.0, o2afr))

        # --- Fuel slowly decreases ---
        fuel -= 0.002
        if fuel < 5:
            fuel = 75  # refill

        # --- MAP from throttle ---
        map_val = 25 + (throttle / 100.0) * 200 + random.uniform(-5, 5)

        # --- Oil pressure from RPM ---
        oil_pressure = 10 + (rpm / 7000) * 50 + random.uniform(-2, 2)
        oil_pressure = max(5, min(80, oil_pressure))

        # --- Build JSON ---
        doc = {
            "event": "sensors",
            "data": {
                "speed": int(speed),
                "rpm": int(rpm),
                "coolant": int(coolant),
                "throttle": int(throttle),
                "map": int(map_val),
                "fuel": int(fuel),
                "oilPressure": int(oil_pressure),
                "oil": int(oil),
                "ambient": 35,
                "intake": int(intake),
                "o2AFR": round(o2afr, 1),
                "driverDoor": False,
                "passengerDoor": False,
                "trunk": False,
                "hood": False
            }
        }

        payload = json.dumps(doc)
        sock.sendto(payload.encode(), (TARGET, PORT))

        if tick % 25 == 0:
            print(f"  RPM={int(rpm):5d}  SPD={speed:5.1f}  THR={int(throttle):3d}%  "
                  f"CLT={coolant:5.1f}  OIL={oil:5.1f}  AFR={o2afr:.1f}  "
                  f"GEAR={gear}  FUEL={int(fuel)}%")

        tick += 1
        time.sleep(INTERVAL / 1000.0)

    except KeyboardInterrupt:
        print("\nStopped.")
        running = False
        break

sock.close()
