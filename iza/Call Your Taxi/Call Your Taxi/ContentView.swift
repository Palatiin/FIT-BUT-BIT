//
//  ContentView.swift
//  Call Your Taxi
//
//  Created by Matúš Remeň on 18/05/2022.
//

import MapKit
import SwiftUI
import SwiftSMTP
import CoreData

// main view - Map + button trio (History, Call, Settings)
struct ContentView: View {
    @Environment(\.managedObjectContext) private var viewContext
     
    @StateObject private var viewModel = ContentViewModel()
    
    @State var emailSubject = "CYTApp: Notification"
    @State var emailBody = "Yo wassup. I need a ride."
    
    var body: some View {
        let emailSubjectBinding = Binding(get: { emailSubject }) {
            emailSubject = $0
        }
        let emailBodyBinding = Binding(get: { emailBody }) {
            emailBody = $0
        }
        
        NavigationView {
            ZStack {
                // background color
                Color("Gray").edgesIgnoringSafeArea(.all)
            
                VStack{
                    Map(
                        coordinateRegion: $viewModel.region,
                        interactionModes: .all,
                        showsUserLocation: true,
                        userTrackingMode: .constant(.follow)
                    )
                        .ignoresSafeArea()
                        .accentColor(Color(.systemRed))
            
                    HStack {
                        Spacer()
                    
                        NavigationLink(destination: LocationsListView(), label: {
                            Text("History")
                        })
                    
                        Spacer()
                    
                        Button(action: addItem) {
                            Text("Call")
                                .bold()
                                .frame(width: 70, height: 70, alignment: .center)
                                .background(Color.red)
                                .cornerRadius(20)
                                .foregroundColor(Color.white)
                        }
                    
                        Spacer()
                    
                        NavigationLink(destination:
                            SettingsView(emailSubject: emailSubjectBinding, emailBody: emailBodyBinding),
                            label: {
                            Text("Settings")
                        })
                    
                        Spacer()
                    }
                }
            }
        }
        .onAppear {
            viewModel.checkIfLocationServicesIsEnabled()
        }
    }

    // functionality of 'Call' button, saves current location and notifies recipient
    private func addItem() {
        withAnimation {
            let newItem = Item(context: viewContext)
            newItem.timestamp = Date()
            newItem.location = "Lat: \(viewModel.region.center.latitude)\nLong: \(viewModel.region.center.longitude)"

            do {
                try viewContext.save()
            } catch {
                print("addItem: Didn't work")
            }
        }
        
        sendMail()
    }

    // sent notification email to receiver (Mail.User)
    private func sendMail() {
        // do not change =
        let smtp = SMTP(
            hostname: "smtp.gmail.com",
            email: "cytapp.notifcation@gmail.com",
            password: "eiokorpdqbmiesbl"
        )
        
        let app = Mail.User(
            name: "CYT App",
            email: "cytapp.notification@gmail.com"
        )
        // ===============
        
        // change to see it in your mailbox
        let receiver = Mail.User(
            name: "Matúš Remeň",
            email: "xremen01@stud.fit.vutbr.cz"
        )
        
        // compose and send email notification
        let mail = Mail(
            from: app,
            to: [receiver],
            subject: emailSubject,
            text: "\(emailBody)\nLat: \(viewModel.region.center.latitude)\nLong: \(viewModel.region.center.longitude)"
        )
        smtp.send(mail) { (error) in
            if let error = error {
                print(error)
            } else {
                print("Send email successful")
            }
        }
    }
}

// show history of calls - timestamp, location
struct LocationsListView: View {
    @Environment(\.managedObjectContext) private var viewContext

    @FetchRequest(
        sortDescriptors: [NSSortDescriptor(keyPath: \Item.timestamp, ascending: true)],
        animation: .default)
    private var items: FetchedResults<Item>
    
    var body: some View {
        List {
            ForEach(items, id: \.self) { (item: Item) in
                NavigationLink(destination: {
                    VStack {
                        // Timestamp
                        HStack {
                            Text("Timestamp:").font(.title)
                            Spacer()
                        }
                        if let _tstamp = item.timestamp {
                            Text("\(_tstamp, formatter: itemFormatter)")
                        }
                        
                        Spacer()
                        
                        // Coordinates
                        HStack{
                            Text("Coords:").font(.title)
                            Spacer()
                        }
                        if let _loc = item.location {
                            Text(_loc)
                        }
                        
                        Spacer()
                        Spacer()
                    }
                }, label: {
                    if let _tstamp = item.timestamp {
                        Text("Item at \(_tstamp, formatter: itemFormatter)")
                    } else {
                        Text("Item at 'Missing timestamp'")
                    }
                })
            }
            .onDelete(perform: deleteItems)
        }
        .toolbar {
            ToolbarItem(placement: .navigationBarTrailing) {
                EditButton()
            }
        }
    }
    
    // delete items from history - db
    private func deleteItems(offsets: IndexSet) {
        withAnimation {
            offsets.map { items[$0] }.forEach(viewContext.delete)

            do {
                try viewContext.save()
            } catch {
                print("deleteItems: Didn't work")
            }
        }
    }
}

// format datetime
private let itemFormatter: DateFormatter = {
    let formatter = DateFormatter()
    
    formatter.dateStyle = .short
    formatter.timeStyle = .medium
    
    return formatter
}()

// edit the notification subject and body
struct SettingsView: View {
    var emailSubject: Binding<String>
    var emailBody: Binding<String>
    
    var body: some View {
        VStack {
            HStack {
                Text("Notification subject:").font(.title)
                Spacer()
            }
            TextEditor(text: emailSubject)
                .font(.body)
                .padding(5)
            
            HStack {
                Text("Notification body:").font(.title)
                Spacer()
            }
            
            TextEditor(text: emailBody)
                .font(.body)
                .padding(5)
            
            Spacer()
            Image("taxicar")
                .resizable()
                .frame(width: 300, height: 250, alignment: .center)
        }
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView().environment(\.managedObjectContext, PersistenceController.preview.container.viewContext)
    }
}

// checking location services authorization and current position
final class ContentViewModel: NSObject, ObservableObject, CLLocationManagerDelegate {
    
    @Published var region = MKCoordinateRegion(
        center: CLLocationCoordinate2D(
            latitude: 49.2,
            longitude: 16.6
        ),
        span: MKCoordinateSpan(
            latitudeDelta: 10,
            longitudeDelta: 10
        )
    )
    
    var locationManager: CLLocationManager?
    
    func checkIfLocationServicesIsEnabled() {
        if CLLocationManager.locationServicesEnabled() {
            locationManager = CLLocationManager()
            guard let _ = locationManager else {return}
            locationManager!.delegate = self
        } else {
            print("Turn location services ON plz.")
        }
    }
    
    private func checkLocationAuthorization() {
        guard let locationManager = locationManager else { return }
        
        switch locationManager.authorizationStatus {
        case .notDetermined:
            locationManager.requestWhenInUseAuthorization()
        case .restricted:
            print("Restricted Auth")
        case .denied:
            print("Denied Auth")
        case .authorizedAlways, .authorizedWhenInUse:
            guard let location = locationManager.location else { return }
            region = MKCoordinateRegion(
                center: location.coordinate,
                span: MKCoordinateSpan(latitudeDelta: 1, longitudeDelta: 1)
            )
        @unknown default:
            break
        }
    }
    
    func locationManagerDidChangeAuthorization(_ manager: CLLocationManager) {
        checkLocationAuthorization()
    }
}
