//
//  NavigationBar.swift
//  hm19
//
//  Created by Saanvi Singh on 12/02/24.
//
//
import Foundation

import CoreBluetooth

import SwiftUI

extension View {
    func navigationBarColor(backgroundColor: Color, titleColor: Color) -> some View {
        modifier(NavigationBarModifier(backgroundColor: backgroundColor, titleColor: titleColor))
    }
}

struct NavigationBarModifier: ViewModifier {
    var backgroundColor: Color
    var titleColor: Color

    init(backgroundColor: Color, titleColor: Color) {
        self.backgroundColor = backgroundColor
        self.titleColor = titleColor

        let coloredAppearance = UINavigationBarAppearance()
        coloredAppearance.configureWithOpaqueBackground()
        coloredAppearance.backgroundColor = UIColor(backgroundColor)
        coloredAppearance.titleTextAttributes = [.foregroundColor: UIColor(titleColor)]
        coloredAppearance.largeTitleTextAttributes = [.foregroundColor: UIColor(titleColor)]

        UINavigationBar.appearance().standardAppearance = coloredAppearance
        UINavigationBar.appearance().scrollEdgeAppearance = coloredAppearance
    }

    func body(content: Content) -> some View {
        content
    }
}

