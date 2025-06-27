
import SwiftUI

struct CommandItem: Identifiable {
    let id = UUID()
    let name: String
    let value: String
}

struct ContentView: View {
    @StateObject var bluetoothManager = BluetoothManager()

    let commands: [CommandItem] = [
        CommandItem(name: "Start Preview", value: "1"),
        CommandItem(name: "Stop Preview", value: "2"),
        CommandItem(name: "Start Data Collection", value: "3"),
        CommandItem(name: "Stop Data Collection", value: "4"),
        CommandItem(name: "Start Inference", value: "5"),
        CommandItem(name: "Stop Inference", value: "6")
    ]

    @State private var selectedCommandValue: String

    init() {
        _bluetoothManager = StateObject(wrappedValue: BluetoothManager())
        _selectedCommandValue = State(initialValue: commands[0].value)
    }

    var body: some View {
        NavigationView {
            VStack {
                Text("Connection Status: \(bluetoothManager.connectionStatus)")
                    .font(.headline)
                    .padding()

                if bluetoothManager.connectedPeripheral == nil {
                    List(bluetoothManager.discoveredPeripherals, id: \.identifier) { peripheral in
                        Button(action: {
                            bluetoothManager.connect(peripheral: peripheral)
                        }) {
                            Text(peripheral.name ?? "Unknown Device")
                        }
                    }
                } else {
                    Text("Connected to: \(bluetoothManager.connectedPeripheral?.name ?? "Unknown")")
                        .font(.subheadline)
                        .padding(.bottom, 5)

                    Button("Disconnect") {
                        bluetoothManager.disconnect()
                    }
                    .padding(.bottom)

                    Divider()

                    // Command Input
                    VStack {
                        Picker("Select Command", selection: $selectedCommandValue) {
                            ForEach(commands) { command in
                                Text(command.name)
                                    .tag(command.value)
                            }
                        }
                        .pickerStyle(.menu)
                        .padding(.horizontal)

                        Button("Send Selected Command") {
                            bluetoothManager.sendCommand(selectedCommandValue)
                        }
                        .padding(.bottom)
                    }
                    .padding()

                    Divider()

                    // Received Messages
                    ScrollView {
                        VStack(alignment: .leading) {
                            ForEach(bluetoothManager.receivedMessages, id: \.self) { message in
                                Text(message)
                                    .font(.caption)
                                    .padding(.vertical, 1)
                            }
                        }
                        .frame(maxWidth: .infinity, alignment: .leading)
                        .padding()
                    }
                }

                Spacer()
            }
            .navigationTitle("BLE Manager")
            .onAppear {
                // Start scanning when the view appears if Bluetooth is powered on
                if bluetoothManager.centralManager.state == .poweredOn {
                    bluetoothManager.startScanning()
                }
            }
        }
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
