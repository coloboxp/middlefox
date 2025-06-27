

import CoreBluetooth
import Foundation

class BluetoothManager: NSObject, ObservableObject, CBCentralManagerDelegate, CBPeripheralDelegate {
    @Published var centralManager: CBCentralManager!
    @Published var discoveredPeripherals: [CBPeripheral] = []
    @Published var connectedPeripheral: CBPeripheral?
    @Published var connectionStatus: String = "Disconnected"
    @Published var receivedMessages: [String] = []

    // MARK: - UUIDs
    let SERVICE_UUID = CBUUID(string: "4fafc201-1fb5-459e-8fcc-c5c9c331914b")
    let CONTROL_CHAR_UUID = CBUUID(string: "beb5483e-36e1-4688-b7f5-ea07361b26a8")
    let STATUS_CHAR_UUID = CBUUID(string: "beb5483e-36e1-4688-b7f5-ea07361b26a9")
    let PREVIEW_INFO_CHAR_UUID = CBUUID(string: "beb5483e-36e1-4688-b7f5-ea07361b26aa")
    let MENU_CHAR_UUID = CBUUID(string: "beb5483e-36e1-4688-b7f5-ea07361b26ab")
    let SERVICE_STATUS_CHAR_UUID = CBUUID(string: "beb5483e-36e1-4688-b7f5-ea07361b26ad")
    let SERVICE_METRICS_CHAR_UUID = CBUUID(string: "beb5483e-36e1-4688-b7f5-ea07361b26ac")

    var controlCharacteristic: CBCharacteristic?
    var statusCharacteristic: CBCharacteristic?
    var previewInfoCharacteristic: CBCharacteristic?
    var menuCharacteristic: CBCharacteristic?
    var serviceStatusCharacteristic: CBCharacteristic?
    var serviceMetricsCharacteristic: CBCharacteristic?

    override init() {
        super.init()
        centralManager = CBCentralManager(delegate: self, queue: nil)
    }

    func startScanning() {
        print("Starting scan for service: \(SERVICE_UUID.uuidString)")
        centralManager.scanForPeripherals(withServices: [SERVICE_UUID], options: nil)
        connectionStatus = "Scanning..."
    }

    func stopScanning() {
        centralManager.stopScan()
        print("Stopped scanning.")
        if connectedPeripheral == nil {
            connectionStatus = "Disconnected"
        }
    }

    func connect(peripheral: CBPeripheral) {
        centralManager.stopScan()
        connectedPeripheral = peripheral
        connectedPeripheral?.delegate = self
        centralManager.connect(peripheral, options: nil)
        connectionStatus = "Connecting to \(peripheral.name ?? "Unknown Device")..."
    }

    func disconnect() {
        if let peripheral = connectedPeripheral {
            centralManager.cancelPeripheralConnection(peripheral)
        }
        connectedPeripheral = nil
        connectionStatus = "Disconnected"
        discoveredPeripherals.removeAll()
        receivedMessages.removeAll()
        controlCharacteristic = nil
        statusCharacteristic = nil
        previewInfoCharacteristic = nil
        menuCharacteristic = nil
        serviceStatusCharacteristic = nil
        serviceMetricsCharacteristic = nil
        print("Disconnected from device.")
    }

    // MARK: - CBCentralManagerDelegate
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        switch central.state {
        case .poweredOn:
            print("Bluetooth is powered on.")
            startScanning()
        case .poweredOff:
            print("Bluetooth is powered off.")
            connectionStatus = "Bluetooth Off"
            discoveredPeripherals.removeAll()
        case .resetting:
            print("Bluetooth is resetting.")
            connectionStatus = "Bluetooth Resetting"
        case .unauthorized:
            print("Bluetooth is unauthorized.")
            connectionStatus = "Bluetooth Unauthorized"
        case .unknown:
            print("Bluetooth state is unknown.")
            connectionStatus = "Bluetooth Unknown"
        case .unsupported:
            print("Bluetooth is unsupported on this device.")
            connectionStatus = "Bluetooth Unsupported"
        @unknown default:
            print("A new Bluetooth state is available.")
            connectionStatus = "Bluetooth Unknown State"
        }
    }

    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        if !discoveredPeripherals.contains(where: { $0.identifier == peripheral.identifier }) {
            discoveredPeripherals.append(peripheral)
            print("Discovered peripheral: \(peripheral.name ?? "Unknown Device")")
        }
    }

    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        print("Connected to \(peripheral.name ?? "Unknown Device")")
        connectionStatus = "Connected to \(peripheral.name ?? "Unknown Device")"
        peripheral.discoverServices([SERVICE_UUID])
    }

    func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: Error?) {
        print("Failed to connect to \(peripheral.name ?? "Unknown Device"): \(error?.localizedDescription ?? "Unknown error")")
        connectionStatus = "Failed to connect"
        disconnect()
    }

    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        print("Disconnected from \(peripheral.name ?? "Unknown Device"): \(error?.localizedDescription ?? "Unknown error")")
        disconnect()
    }

    // MARK: - CBPeripheralDelegate
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        guard let services = peripheral.services else { return }
        for service in services {
            print("Discovered service: \(service.uuid.uuidString)")
            peripheral.discoverCharacteristics(nil, for: service) // Discover all characteristics for the service
        }
    }

    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        guard let characteristics = service.characteristics else { return }
        for characteristic in characteristics {
            print("Discovered characteristic: \(characteristic.uuid.uuidString) with properties: \(characteristic.properties.rawValue)")

            if characteristic.uuid == CONTROL_CHAR_UUID {
                controlCharacteristic = characteristic
                print("Found Control Characteristic")
            } else if characteristic.uuid == STATUS_CHAR_UUID {
                statusCharacteristic = characteristic
                peripheral.setNotifyValue(true, for: characteristic)
                print("Found Status Characteristic, subscribed to notifications.")
            } else if characteristic.uuid == PREVIEW_INFO_CHAR_UUID {
                previewInfoCharacteristic = characteristic
                peripheral.setNotifyValue(true, for: characteristic)
                print("Found Preview Info Characteristic, subscribed to notifications.")
            } else if characteristic.uuid == MENU_CHAR_UUID {
                menuCharacteristic = characteristic
                peripheral.readValue(for: characteristic) // Read the menu characteristic
                print("Found Menu Characteristic, reading value.")
            } else if characteristic.uuid == SERVICE_STATUS_CHAR_UUID {
                serviceStatusCharacteristic = characteristic
                peripheral.setNotifyValue(true, for: characteristic)
                print("Found Service Status Characteristic, subscribed to notifications.")
            } else if characteristic.uuid == SERVICE_METRICS_CHAR_UUID {
                serviceMetricsCharacteristic = characteristic
                peripheral.setNotifyValue(true, for: characteristic)
                print("Found Service Metrics Characteristic, subscribed to notifications.")
            }
        }
    }

    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        if let error = error {
            print("Error updating value for characteristic \(characteristic.uuid.uuidString): \(error.localizedDescription)")
            return
        }

        guard let value = characteristic.value else { return }
        let message = String(data: value, encoding: .utf8) ?? "Invalid data"
        print("Received from \(characteristic.uuid.uuidString): \(message)")

        DispatchQueue.main.async {
            self.receivedMessages.append("[" + characteristic.uuid.uuidString + "] \(message)")
        }
    }

    func peripheral(_ peripheral: CBPeripheral, didWriteValueFor characteristic: CBCharacteristic, error: Error?) {
        if let error = error {
            print("Error writing value for characteristic \(characteristic.uuid.uuidString): \(error.localizedDescription)")
        } else {
            print("Successfully wrote value to characteristic \(characteristic.uuid.uuidString)")
        }
    }

    // MARK: - Command Sending
    func sendCommand(_ command: String) {
        guard let peripheral = connectedPeripheral,
              let controlChar = controlCharacteristic else {
            print("Not connected to a peripheral or control characteristic not found.")
            return
        }

        guard let data = command.data(using: .utf8) else {
            print("Failed to convert command to data.")
            return
        }

        peripheral.writeValue(data, for: controlChar, type: .withResponse)
        print("Sent command: \(command)")
    }
}
