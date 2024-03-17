import SwiftUI

struct ContentView: View {
    
    @State var completedAnimation = false
    @State var isSceneSelected = false
    @State var sceneFile: URL?
    @State private var showFileImporter = false
    
    var body: some View {
        if !completedAnimation {
            SplashScreenView(completedAnimation: $completedAnimation)
        } else {
            GeometryReader { geometry in
                ZStack {
                    Color(hex: 0x0f032e, alpha: 1.0)
                        .edgesIgnoringSafeArea(.all)
                    
                    Image("logo") // Replace "your-logo" with your actual logo image
                        .resizable()
                        .aspectRatio(contentMode: .fit)
                        .frame(width: 200, height: 200)
                        .clipShape(Circle())
                        .offset(y: -geometry.size.height / 3)
                    
                    Text("VulkanSplatting")
                        .font(.title)
                        .fontWeight(.bold)
                        .foregroundColor(.white)
                        .offset(y: -geometry.size.height / 3 + 100) // Adjust positioning as needed
                    
                    VStack {
                        Button(action: {
                            showFileImporter = true
                        }) {
                            VStack {
                                Image(systemName: "square.and.arrow.down")
                                    .font(.largeTitle) // Adjust the size as needed
                                
                                Text("Open scene")
                                    .fontWeight(.medium)
                            }
                            .foregroundColor(Color(hex: 0xE0E0E0)) // Sets the color of the text and icon
                            .padding() // Adds space around the icon and text
                            .background(
                                RoundedRectangle(cornerSize: CGSize(width: 8, height: 8)) // Rounded rectangle background
                                    .stroke(style: StrokeStyle(lineWidth: 2, dash: [10])) // Dashed border
                                    .foregroundColor(Color(hex: 0xE0E0E0))
                            )
                        }
                        
                        Spacer()
                        
                        Link(destination: URL(string: "https://github.com/shg8/VulkanSplatting")!) {
                            Label {
                                Text("Leave a star!")
                                    .font(.subheadline.lowercaseSmallCaps())
                            } icon: {
                                Image("github-mark-white")
                                    .resizable()
                                    .frame(width: 20, height: 20)
                            }
                        }.padding()
                    }
                    .frame(minWidth: 0, maxWidth: .infinity, minHeight: 0, maxHeight: geometry.size.height / 2)
                    .offset(y: geometry.size.height / 4)
                    .edgesIgnoringSafeArea(.all)
                    .fileImporter(
                        isPresented: $showFileImporter,
                        allowedContentTypes: [.data],
                        allowsMultipleSelection: false
                    ) { result in
                        switch result {
                        case .success(let files):
                            files.forEach { file in
                                // gain access to the directory
                                let gotAccess = file.startAccessingSecurityScopedResource()
                                if !gotAccess { return }
                                // access the directory URL
                                // (read templates in the directory, make a bookmark, etc.)
                                sceneFile = file
                                isSceneSelected = true
                                //                    handlePickedPDF(file)
                                // release access
                                //                    file.stopAccessingSecurityScopedResource()
                            }
                        case .failure(let error):
                            // handle error
                            print(error)
                        }
                    }.fullScreenCover(isPresented: $isSceneSelected) {
                        VulkanView(sceneFile: $sceneFile)
                            .frame(minWidth: 0, maxWidth: .infinity, minHeight: 0, maxHeight: .infinity)
                            .edgesIgnoringSafeArea(.all)
                            .overlay(alignment: .topLeading) {
                                CloseButton(action: {
                                    isSceneSelected = false
                                })
                                .padding()
                                .frame(maxWidth: .infinity, alignment: .topLeading)
                            }
                    }
                }.preferredColorScheme(.dark)
            }.frame(maxWidth: .infinity, maxHeight: .infinity)
                .ignoresSafeArea()
        }
    }
}

struct CloseButton: View {
    var action: () -> Void
    
    var body: some View {
        Button(action: action) {
            Image(systemName: "xmark.circle.fill")
                .font(.title)
                .foregroundColor(.white)
        }
        .padding(10)
        .background(Color.black.opacity(0.5))
        .clipShape(Circle())
    }
}
