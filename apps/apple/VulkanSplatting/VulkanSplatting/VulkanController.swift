//
//  VulkanController.swift
//  VulkanSplatting
//
//  Created by Steven on 3/14/24.
//

import SwiftUI
import UIKit

class MetalView: UIView {
    override class var layerClass: AnyClass {
        return CAMetalLayer.self
    }
}

class VulkanController: UIViewController {
    
    var sceneFile: URL?
    var panGesture: UIPanGestureRecognizer!
    var moveGesture: UIPanGestureRecognizer!
    var pinchGesture: UIPinchGestureRecognizer!
    var displayLink: CADisplayLink!
    
    override func loadView() {
        view = MetalView()
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        self.view.contentScaleFactor = UIScreen.main.nativeScale;
        
        setupInput()
        
        displayLink = CADisplayLink(target: self, selector: #selector(self.renderLoop))
        displayLink.preferredFrameRateRange = CAFrameRateRange(minimum: 60.0, maximum: 60.0, preferred: 60.0)
    }
    
    func getCurrentOrientationMask() -> UIInterfaceOrientationMask {
        switch UIDevice.current.orientation {
        case .portrait:
            return .portrait
        case .landscapeRight:
            return .landscapeRight
        case .landscapeLeft:
            return .landscapeLeft
        case .portraitUpsideDown:
            return .portraitUpsideDown
        default:
            return .portrait
        }
    }
    
    override func viewDidAppear(_ animated: Bool) {
        // lock orientaion
        VulkanSplattingApp.orientationLock = getCurrentOrientationMask()
        let metalLayer = self.view.layer as! CAMetalLayer
        
        var config = VulkanSplatting.RendererConfiguration()
        
        config.scene = std.string(sceneFile!.path(percentEncoded: false))
        config.window = VulkanSplatting.createMetalWindow(Unmanaged.passUnretained(metalLayer).toOpaque(), Int32(UIScreen.main.nativeBounds.width), Int32(UIScreen.main.nativeBounds.height))
        
        vkgs_initialize(config)
        displayLink.add(to: .current, forMode: .default)
    }
    
    override func viewDidDisappear(_ animated: Bool) {
        displayLink.remove(from: .current, forMode: .default)
        vkgs_cleanup()
        VulkanSplattingApp.orientationLock = .all
    }
    
    @objc func handlePanGesture(_ recognizer: UIPanGestureRecognizer) {
        let translation = recognizer.translation(in: self.view)
        
        // Reset the translation to zero after it's been used so the next callback gives incremental changes
        recognizer.setTranslation(CGPoint.zero, in: self.view)
        vkgs_pan_translation(Float(translation.x), Float(translation.y))
    }
    
    @objc func handleMoveGesture(_ recognizer: UIPanGestureRecognizer) {
        let translation = recognizer.translation(in: self.view)
        
        recognizer.setTranslation(CGPoint.zero, in: self.view)
        vkgs_movement(Float(translation.x * -0.01), Float(translation.y * 0.01), 0)
    }
    
    @objc func handlePinch(_ gesture: UIPinchGestureRecognizer) {
        if gesture.state == .changed {
            // Determine whether the user is zooming in or out
            let scale = gesture.scale
            if scale > 1.0 {
                // Zooming in
                vkgs_movement(0, 0, -Float(scale - 1))
            } else {
                // Zooming out
                vkgs_movement(0, 0, 1-Float(scale))
            }
            
            // Reset the scale to avoid accumulating the scale factor
            gesture.scale = 1.0
        }
    }
    
    
    @objc func renderLoop() {
        print("renderLoop")
        vkgs_draw()
    }
    
    func setupInput() {
        panGesture = UIPanGestureRecognizer(target: self, action: #selector(handlePanGesture(_:)))
        panGesture.maximumNumberOfTouches = 1
        panGesture.minimumNumberOfTouches = 1
        self.view.addGestureRecognizer(panGesture)
        
        moveGesture = UIPanGestureRecognizer(target: self, action: #selector(handleMoveGesture(_:)))
        moveGesture.maximumNumberOfTouches = 2
        moveGesture.minimumNumberOfTouches = 2
        self.view.addGestureRecognizer(moveGesture)
        
        pinchGesture = UIPinchGestureRecognizer(target: self, action: #selector(handlePinch(_:)))
        self.view.addGestureRecognizer(pinchGesture)
    }
}

struct VulkanView: UIViewControllerRepresentable {
    
    @Binding var sceneFile: URL?
    
    func updateUIViewController(_ uiViewController: VulkanController, context: Context) {
        
    }
    
    func makeUIViewController(context: Context) -> VulkanController {
        var controller = VulkanController()
        controller.sceneFile = sceneFile
        return controller
    }
}
