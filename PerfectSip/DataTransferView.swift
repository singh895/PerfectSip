////
////  DataTransferView.swift
////  hm19
////
////  Created by Saanvi Singh on 12/02/24.
////
/////
import SwiftUI
import CoreBluetooth
import Foundation
//
struct DataTransferView: View {
    @ObservedObject var bluetoothManager: BluetoothManager
    @State private var userInput: String = ""
    @State private var sliderValue = 135.0
    
    var body: some View {
        VStack {
            
            Text("Set Temperature")
                .font(.title)
                .padding()
            
            Text("Select Desired Temperature")
                .padding()
            
            ZStack(alignment: .leading) {
               LinearGradient(gradient: Gradient(colors: [.yellow, .red]), startPoint: .leading, endPoint: .trailing)
                   .frame(height: 4) // Adjust the height of the track
                   .cornerRadius(2) // Round the corners of the track

               Slider(value: $sliderValue, in: 120...150, step: 1) // Slider from 120 to 150
                   .accentColor(.clear) // Make the thumb color clear
                   
           }
            Text("\(Int(sliderValue))")
                            .padding()
            Button("Send Data") {
                bluetoothManager.sendData(String(format: "%.0f\n", sliderValue)) // Convert slider value to string
            }
            .padding()
            
            Divider()
            
            Text("Current Beverage Temperature: \(bluetoothManager.temp)")
                .padding()
        }
    }
}

