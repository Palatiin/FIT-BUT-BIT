//
//  Call_Your_TaxiApp.swift
//  Call Your Taxi
//
//  Created by Matúš Remeň on 18/05/2022.
//

import SwiftUI

@main
struct Call_Your_TaxiApp: App {
    let persistenceController = PersistenceController.shared

    var body: some Scene {
        WindowGroup {
            ContentView()
                .environment(\.managedObjectContext, persistenceController.container.viewContext)
        }
    }
}
