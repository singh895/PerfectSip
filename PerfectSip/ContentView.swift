//
//  ContentView.swift
//  hm19
//
//  Created by Saanvi Singh on 07/02/24.
//
//import SwiftUI
//import CoreBluetooth
//
//struct ContentView: View {
//    @StateObject var bluetoothManager = BluetoothManager()
//    @State private var dataToSend: String = ""
//    @State private var sliderValue = 135.0
//    var body: some View {
//        VStack {
////            Image(systemName: "globe")
////                .imageScale(.large)
////                .foregroundStyle(.tint)
////                .padding()
//            List(bluetoothManager.peripherals, id: \.self) { peripheral in
//                Text(peripheral.name ?? "Unnamed Peripheral")
//
//            }
//            Text(bluetoothManager.connectionStatus)
//                .foregroundColor(bluetoothManager.isConnected ? .green : .red)
//                .padding()
//
//
//            ZStack(alignment: .leading) {
//                LinearGradient(gradient: Gradient(colors: [.yellow, .red]), startPoint: .leading, endPoint: .trailing)
//                    .frame(height: 4) // Adjust the height of the track
//                    .cornerRadius(2)
//                Slider(value: $sliderValue, in: 120...150, step: 1) // Slider from 120 to 150
//                    .padding()
//                    .foregroundColor(.red)
//                    .accentColor(.clear)
//
//            }
//
//            Text("Desired Temperature: \(Int(sliderValue))")
//                .padding()
//            Text(bluetoothManager.dataSentStatus)
//                .foregroundColor(bluetoothManager.isDataSent ? .green : .red)
//                .padding()
//            Text(bluetoothManager.temp)
//                .foregroundColor( .green)
//                .padding()
//
//
//        }
//        .onAppear {
//            bluetoothManager.startBluetoothOperations()
//            bluetoothManager.sendData(sliderValue: Int(sliderValue))
//        }
//    }
//}
//
//struct ContentView_Previews: PreviewProvider {
//    static var previews: some View {
//        ContentView()
//    }
//}
//
//class BluetoothManager: NSObject, CBCentralManagerDelegate, CBPeripheralDelegate, ObservableObject {
//    var centralManager: CBCentralManager!
//    var peripheral: CBPeripheral?
//    let hm19UUID = CBUUID(string: "FFE0")
//    let characteristicUUID = CBUUID(string: "FFE1")
//
//    @Published var peripherals: [CBPeripheral] = []
//    @Published var peripheralNames: [String] = []
//
//    @Published var connectionStatus = "Not Connected"
//    @Published var dataSentStatus = "Data Not Sent"
//    @Published var temp = "Received data:"
//    @Published var sliderValue = 135
//
//    var isConnected = false
//    var isDataSent = false
//
//    var characteristic: CBCharacteristic?
//
//    override init() {
//        super.init()
//        centralManager = CBCentralManager(delegate: self, queue: nil)
//    }
//
//    func startBluetoothOperations() {
//        centralManager = CBCentralManager(delegate: self, queue: nil)
//    }
//
//    func centralManagerDidUpdateState(_ central: CBCentralManager) {
//        if central.state == .poweredOn {
//            centralManager.scanForPeripherals(withServices: [hm19UUID], options: nil)
//            print("Bluetooth is powered on!")
//        } else {
//            print("Bluetooth is not available")
//        }
//    }
//
//
//    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
//            print("Discovered peripheral: \(peripheral)")
//            DispatchQueue.main.async { [weak self] in
//                guard let self = self else { return }
//                if let peripheralName = peripheral.name, !peripheralName.isEmpty {
//                    if !self.peripherals.contains(peripheral) {
//                        self.peripherals.append(peripheral)
//                        self.peripheralNames.append(peripheralName)
//                    }
//                }
//            }
//            // Connect to the discovered peripheral
//            self.peripheral = peripheral
//            self.peripheral?.delegate = self
//            centralManager.connect(peripheral, options: nil)
//        }
//
//    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
//        print("Connected to peripheral: \(peripheral)")
//        connectionStatus = "Connected to \(peripheral.name ?? "nothing")"
//        isConnected = true
//        peripheral.discoverServices(nil)
//    }
//
//    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
//        guard let services = peripheral.services else { return }
//        for service in services {
//            peripheral.discoverCharacteristics(nil, for: service)
//        }
//    }
//
////    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
////        guard let characteristics = service.characteristics else { return }
////        for characteristic in characteristics {
////            if characteristic.uuid == characteristicUUID {
////                let userInput = "\(Int(sliderValue))"
////                if let dataToSend = userInput.data(using: .utf8) {
////
////                    peripheral.writeValue(dataToSend, for: characteristic, type: .withResponse)
////                    dataSentStatus = "Data Sent: \(userInput)"
////                    isDataSent = true
////
////                } else {
////                    print("Failed to convert user input to data")
////                    dataSentStatus = "Data Send Failed"
////                }
////                peripheral.setNotifyValue(true, for: characteristic)
////            }
////        }
////    }
//    func sendData(sliderValue: Int) {
//        guard let peripheral = peripheral, let characteristic = characteristic else {
//            print("Peripheral or characteristic is not available")
//            return
//        }
//
//        let dataToSend = "\(sliderValue) ".data(using: .utf8)!
//        peripheral.writeValue(dataToSend, for: characteristic, type: .withResponse)
//        dataSentStatus = "Data Sent: \(sliderValue)"
//        isDataSent = true
//    }
//
//    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
//        if let data = characteristic.value {
//            if let receivedString = String(data: data, encoding: .utf8) {
//                print("Received data: \(receivedString)")
//                temp = receivedString
//            } else {
//                print("Unable to received data")
//            }
//        }
//    }
//}


import SwiftUI
import CoreBluetooth
import UserNotifications


class BluetoothManager: NSObject, CBCentralManagerDelegate, CBPeripheralDelegate, ObservableObject {
    var centralManager: CBCentralManager!
    var peripheral: CBPeripheral?
    let hm19UUID = CBUUID(string: "FFE0")
    let characteristicUUID = CBUUID(string: "FFE1")
    
    @Published var peripherals: [CBPeripheral] = []
    @Published var peripheralNames: [String] = []
    
    @Published var connectionStatus = "Not Connected"
    @Published var dataSentStatus = "Data Not Sent"
    @Published var temp = ""

    var isConnected = false
    var isDataSent = false

    override init() {
        super.init()
        centralManager = CBCentralManager(delegate: self, queue: nil)
    }

    func startBluetoothOperations() {
        centralManager = CBCentralManager(delegate: self, queue: nil)
    }

    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        if central.state == .poweredOn {
            centralManager.scanForPeripherals(withServices: [hm19UUID], options: nil)
            print("Bluetooth is powered on!")
        } else {
            print("Bluetooth is not available")
        }
    }

    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        print("Discovered peripheral: \(peripheral)")
        DispatchQueue.main.async { [weak self] in
            guard let self = self else { return }
            if let peripheralName = peripheral.name, !peripheralName.isEmpty {
                if !self.peripherals.contains(peripheral) {
                    self.peripherals.append(peripheral)
                    self.peripheralNames.append(peripheralName)
                }
            }
        }
        // Connect to the discovered peripheral
        self.peripheral = peripheral
        self.peripheral?.delegate = self
        centralManager.connect(peripheral, options: nil)
    }

    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        print("Connected to peripheral: \(peripheral)")
        connectionStatus = "Connected"
        isConnected = true
        peripheral.discoverServices(nil)
    }

    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        guard let services = peripheral.services else { return }
        for service in services {
            peripheral.discoverCharacteristics(nil, for: service)
        }
    }

    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        guard let characteristics = service.characteristics else { return }
        for characteristic in characteristics {
            if characteristic.uuid == characteristicUUID {
                peripheral.setNotifyValue(true, for: characteristic)
            }
        }
    }

    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        if let data = characteristic.value {
            if let receivedString = String(data: data, encoding: .utf8) {
                print("Received data: \(receivedString)")
                temp = receivedString
            } else {
                print("Unable to receive data")
            }
        }
    }
    
    func sendData(_ data: String) {
        if let peripheral = self.peripheral, let characteristic = peripheral.services?.first?.characteristics?.first {
            if let dataToSend = data.data(using: .utf8) {
                peripheral.writeValue(dataToSend, for: characteristic, type: .withResponse)
                dataSentStatus = "Data Sent Success"
                isDataSent = true
            } else {
                print("Failed to convert user input to data")
                dataSentStatus = "Data Send Failed"
            }
        }
    }
}


//struct ContentView: View {
//    @StateObject var bluetoothManager = BluetoothManager()
//    @State private var sliderValue = 135.0
//    @State private var selectedPeripheral: CBPeripheral?
//    @State private var isShowingDataTransferView = false
//
//
//    var body: some View {
//
//        NavigationView {
////
////               LinearGradient(gradient: Gradient(colors: [.yellow, .red]), startPoint: .leading, endPoint: .trailing)
////                   .frame(height: 4) // Adjust the height of the track
////                   .cornerRadius(2) // Round the corners of the track
////
////               Slider(value: $sliderValue, in: 120...150, step: 1) // Slider from 120 to 150
////                   .accentColor(.clear) // Make the thumb color clear
////
////           }
////            Text("\(Int(sliderValue))")
////                            .padding()
////            Button("Send Data") {
////                bluetoothManager.sendData(String(format: "%.0f", sliderValue)) // Convert slider value to string
////            }
////            .padding()
////
////            Divider()
//            VStack {
//                List(bluetoothManager.peripherals, id: \.self) { peripheral in
//                    Button(action: {
//                        // Check if connected before allowing navigation
//                        if bluetoothManager.isConnected {
//                            selectedPeripheral = peripheral
//                            isShowingDataTransferView = true
//                        }
//                    }) {
//                        Text(peripheral.name ?? "Unnamed Peripheral")
//                    }
//                }
//
//                Text(bluetoothManager.connectionStatus)
//                    .foregroundColor(bluetoothManager.isConnected ? .green : .red)
//                    .padding()
//                Text(bluetoothManager.dataSentStatus)
//                    .foregroundColor(bluetoothManager.isDataSent ? .green : .red)
//                    .padding()
//            }
//            .navigationTitle("Discovered Devices")
//            .navigationBarTitleDisplayMode(.inline)
//            .navigationBarColor(backgroundColor: .blue, titleColor: .white)
//            .sheet(isPresented: $isShowingDataTransferView) {
//                if let selectedPeripheral = selectedPeripheral {
//                    DataTransferView(bluetoothManager: bluetoothManager, peripheral: selectedPeripheral)
//                }
//            }
//        }
//        .onAppear {
//            bluetoothManager.startBluetoothOperations()
//            print(bluetoothManager.isConnected)
//        }
////        .sheet(isPresented: $bluetoothManager.isConnected) {
////            DataTransferView(bluetoothManager: bluetoothManager)
////        }
//    }
//}

struct ContentView: View {
    @StateObject var bluetoothManager = BluetoothManager()
    @State private var selectedPeripheral: CBPeripheral?
    @State private var isShowingDataTransferView = false
    
    var body: some View {
        NavigationView {
            VStack {
                
                List(bluetoothManager.peripherals, id: \.self) { peripheral in
                    Button(action: {
                        // Check if connected before allowing navigation
                        if bluetoothManager.isConnected {
                            selectedPeripheral = peripheral
                            isShowingDataTransferView = true
                        }
                    }) {
                        Text(peripheral.name ?? "Unnamed Peripheral")
                            .foregroundColor(bluetoothManager.isConnected ? .green : .primary)
                    }
                    .disabled(!bluetoothManager.isConnected)
                }
                Image("logo2") // Replace "your_logo_name" with the actual name of your logo image
                    .resizable()
                    .aspectRatio(contentMode: .fit)
                    .frame(width: 1000)
                    .padding()
                Text(bluetoothManager.connectionStatus)
                    .foregroundColor(bluetoothManager.isConnected ? .green : .red)
                    .padding()
                Text(bluetoothManager.dataSentStatus)
                    .foregroundColor(bluetoothManager.isDataSent ? .green : .red)
                    .padding()
            }
            .navigationTitle("Discovered Devices")
            .navigationBarTitleDisplayMode(.inline)
            .navigationBarColor(backgroundColor: .blue, titleColor: .white)
            .sheet(isPresented: $isShowingDataTransferView) {
                if let selectedPeripheral = selectedPeripheral {
                    DataTransferView(bluetoothManager: bluetoothManager)
                }
            }
        }
        .onAppear {
            bluetoothManager.startBluetoothOperations()
        }
    }
}


struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
