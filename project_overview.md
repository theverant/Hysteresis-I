# Hysteresis - When Matter Learns to Remember

**Hysteresis** transforms the secret life of electromagnetic fields into visible entanglement through robotic tentacles that search, discover, and respond to forces around them. This kinetic installation explores how matter remembers—the way magnetic domains hold onto their history, creating behaviors that seem almost... personal.

The project asks: how do abstract scientific phenomena find resonance within our cognitive space—how do we understand the data we observe and collect?  What emerges is a dialogue between light and movement, where servo-controlled appendages conduct intricate signal exchanges guided by the mathematics of magnetic memory.  Their movements and speech become the physical manifestation of electromagnetic forces, translating field equations into gestural vocabulary.

### Conceptual Choreography
- **Electromagnetic Translation**: Converting field equations into gestural vocabulary
- **Memory Made Visible**: Hysteresis curves become patterns of light that linger and fade
- **Mechanical Intimacy**: Servo movements that search, approach, and align with organic curiosity
- **Responsive Ecosystems**: Multiple entities creating emergent conversations through proximity

### Aesthetic Intention
- **Organic Automation**: Tentacles that move like living things discovering their environment
- **Temporal Accumulation**: Complex behaviors building from simple attraction algorithms  
- **Atmospheric Immersion**: LED arrays creating responsive environmental moods that shift with field interactions
- **Patient Narratives**: Slow revelations rewarding extended observation

## Technical Topology

### The Physical Orchestra

#### Movement Architecture
* **Arduino Nano**: The nervous system coordinating behavior
* **Servo Actuators**: X/Y positioning (pins D3, D5) with naturalistic drift patterns
* **Search Algorithms**: Clock-sweep motion layered with Perlin noise for biological authenticity

#### Light Vocabulary
* **WS2812 RGB LEDs**: Three points per tentacle creating field visualization language
* **Dynamic Role Assignment**: LEDs shift functions during operation, like domains reorienting
* **Hysteresis Curves**: 16-step brightness patterns encoding magnetic memory into visual rhythm

#### Environmental Sensing
* **Photodiode Detection**: Single sensor (pin A2) with transimpedance amplification
* **Spectral Decomposition**: Software parsing of RGB signal components into distinct behaviors
* **Adaptive Response**: Real-time adjustment to ambient lighting conditions

#### Human Interface
* **Manual Override**: Joystick control (pins A0, A1, D2) for direct tentacle guidance
* **Serial Communication**: Live parameter adjustment and system monitoring

### Behavioral Architecture

#### State Progression
```
CURIOUS_EXPLORATION → SIGNAL_DETECTION → GRADUAL_APPROACH → FIELD_SYNCHRONIZATION → COORDINATED_DISPLAY
```

#### Movement Vocabularies
- **Discovery Mode**: Wide sweeping explorations with organic unpredictability
- **Attraction Response**: Gradual focus toward detected electromagnetic signatures
- **Convergence Behavior**: Fine positioning for optimal signal alignment
- **Synchronized Performance**: Coordinated movements creating unified light displays

#### Signal Processing
- **Channel Separation**: RGB components trigger different response behaviors
- **Red Frequency**: Long-distance attraction and discovery signaling
- **Green Channel**: Mid-range synchronization and temporal coordination  
- **Blue Spectrum**: Close-range fine-tuning and precise positioning adjustments
- **Brightness Modulation**: Hysteresis curves create organic fade patterns with memory

## Making the Invisible Intimate

### Domain Switching as Performance
The installation materializes magnetic behavior through:
- **LED Color Rotation**: Simulating domain reorientation under field influence
- **Memory Curves**: Non-linear brightness fades reflecting how materials remember
- **Temporal Displacement**: Individual LED timing creates complex interference patterns
- **Variable Rhythms**: Speed modulation simulating changing field strengths

### Pattern Language
```cpp
// Aesthetic DNA controlling visual behavior
const float hysteresisPattern[16] = {
  0.00, 0.05, 0.10, 0.20, 0.30, 0.45, 0.60, 0.75,
  1.00, 0.75, 0.60, 0.45, 0.30, 0.20, 0.10, 0.05
};

const unsigned long fadeSpeedPattern[] = {
  50, 70, 90, 110, 130, 150, 130, 110, 90, 70
};
```

### Environmental Integration
- **Ambient Adaptation**: System adjusts to room lighting for optimal visibility
- **Circadian Rhythms**: Long-period cycles creating evolving atmospheric moods
- **Spatial Relationships**: Multiple tentacles generating complex field interactions
- **Observer Effect**: Presence detection potentially modifying behavior patterns

## Technical Specifications

### Performance Parameters
- **Response Rate**: 20Hz sensor sampling for fluid responsiveness
- **Movement Range**: Complete 360° rotation on both servo axes
- **Detection Radius**: 1-3 meter effective sensing envelope
- **Color Resolution**: 24-bit RGB with 150/255 brightness ceiling for comfort
- **Pattern Memory**: 16-step hysteresis curves with individual LED phase relationships

### Signal Intelligence
- **Spectral Analysis**: Mathematical separation of color components into behavioral triggers
- **Noise Management**: Digital filtering for stable signal detection
- **Threshold Adaptation**: Dynamic sensitivity adjustment to environmental conditions
- **Pattern Recognition**: Correlation analysis identifying specific electromagnetic signatures

### Infrastructure Requirements
- **Power Supply**: 5V/2A for stable LED operation across all channels
- **Data Protocol**: WS2812 serial RGB control with precise timing
- **Signal Amplification**: LM358 op-amp with 1MΩ feedback for photodiode sensitivity
- **Operating Environment**: Standard indoor conditions (15-35°C range)

## Research Territory

### Visualization Methods
This project develops approaches for:
- **Concept Materialization**: Rendering invisible phenomena into tangible experience
- **Behavioral Algorithm Design**: Creating convincingly organic robotic responses
- **Multi-Modal Integration**: Weaving light, movement, and temporal patterns into coherent experience
- **Autonomous Installation Framework**: Self-directing systems that engage observers naturally

### Creative Outcomes
- **Kinetic Light Installation**: Self-contained interactive electromagnetic theater
- **Educational Visualization**: Physical demonstration of abstract scientific principles
- **Performance Integration**: Dynamic lighting element for live presentations
- **Scalable Series**: Framework expandable to multiple interacting units

### Development Horizons
- **Swarm Intelligence**: Multi-tentacle coordination and emergent behaviors
- **Environmental Integration**: Temperature, humidity, sound as input modalities
- **Haptic Simulation**: Physical force representation of magnetic attraction
- **Audio Synthesis**: Sound generation driven by field interaction patterns
- **Network Coordination**: Wireless communication between distributed installations

---

## Implementation Notes

### Code Evolution
Current implementation (v0.15) features:
- Independent LED fade timing with hysteresis-based speed variation
- Encrypted pattern storage using XOR obfuscation techniques
- Smooth color transitions between RGB primaries
- Environmental brightness adaptation for optimal visibility

### Hardware Philosophy
- Single-board control system managing all subsystems efficiently
- Modular sensor architecture enabling future capability expansion
- Standard servo interfaces ensuring reliable mechanical positioning
- Addressable LED arrays providing scalable visual complexity

### Performance Strategy
- Lookup tables replacing computationally expensive mathematical operations
- Efficient bit manipulation for rapid pattern indexing
- Minimal memory footprint optimized for embedded system constraints
- Real-time processing architecture avoiding blocking delays

---

**v2**