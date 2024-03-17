import SwiftUI

struct SplashScreenView: View {
    @Binding var completedAnimation: Bool
    
    // Animation states
    @State private var startAnimation = false
    @State private var expandBackground = false
    @State private var textOpacity = 0.0
    
    var body: some View {
        ZStack {
            // The expanding background
            Color(hex: 0x0f032e, alpha: 1.0)
                .frame(width: expandBackground ? 2000 : 200, height: expandBackground ? 2000 : 200)
                .clipShape(Circle())
                .onAppear {
                    // Start the initial logo animation
                    withAnimation(.easeIn(duration: 0.5)) {
                        startAnimation = true
                        withAnimation(.easeInOut(duration: 0.5)) {
                            expandBackground = true
                        }
                    }
                    // Begin fading in the text slightly after the logo starts moving
                    withAnimation(.easeIn(duration: 0.5).delay(0.25)) {
                        textOpacity = 1.0
                    }
                    // Transition to main content after animations
                    DispatchQueue.main.asyncAfter(deadline: .now() + 1) {
                        completedAnimation = true
                    }
                }
                .offset(y: startAnimation ? -UIScreen.main.bounds.height / 3 : 0)
            
            // Your logo at the center
//            if !completedAnimation {
                Image("logo") // Replace "your-logo" with your actual logo image
                    .resizable()
                    .aspectRatio(contentMode: .fit)
                    .frame(width: 200, height: 200)
                    .clipShape(Circle())
                    .offset(y: startAnimation ? -UIScreen.main.bounds.height / 3 : 0)
                
                // Add this below the Image view for the logo
                Text("VulkanSplatting")
                    .font(.title)
                    .fontWeight(.bold)
                    .foregroundColor(.white)
                    .opacity(textOpacity)
                    .offset(y: startAnimation ? (-UIScreen.main.bounds.height / 3) + 100 : 0) // Adjust positioning as needed
//            }
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .ignoresSafeArea()
    }
}

extension Color {
    init(hex: UInt, alpha: Double = 1) {
        self.init(
            .sRGB,
            red: Double((hex >> 16) & 0xff) / 255,
            green: Double((hex >> 08) & 0xff) / 255,
            blue: Double((hex >> 00) & 0xff) / 255,
            opacity: alpha
        )
    }
}
