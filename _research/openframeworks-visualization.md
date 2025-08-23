---
layout: single
title: "OpenFrameworks Visualization Development"
sidebar:
  nav: "research"
toc: true
toc_label: "Development Topics"
---

*Technical discussion about visualizing tentacle search patterns and building control systems*

## Initial Visualization Requirements

T.W. proposed using OpenFrameworks to visualize the search patterns from the H1XYXY_1_1.ino code, which implements V31 territorial exploration with compound curvature on an Arduino Uno using dual XY servo systems.

OpenFrameworks identified as suitable for:
- Real-time graphics visualization
- Serial data communication with Arduino
- 2D and 3D pattern rendering
- Interactive control system development

## 3D Space Mapping Discussion

**Initial Question:** Mapping 2D servo positions to 3D tentacle space as a function of segment length, knuckle diameter, and filament wheel size.

**Technical Structure Clarification:**
After multiple corrections, the tentacle structure was identified as:
- 8 knuckles total (2 large, 4 medium, 2 small)
- 11mm spacing between each knuckle
- Structure: B-X-X-X-X-X-X-X-X (B=base, X=knuckle, -=11mm spine)
- No spine extending past knuckle 8
- Total length ~25cm

**Control Zones:**
- Base XY controls knuckles 1-4 (2 large + 2 medium using outer thread position)
- Tip XY controls knuckles 5-8 (2 medium using inner thread position + 2 small)

## Kinematic Modeling Approach

**Forward Kinematics Concept:**
Each knuckle creates a bend point influenced by cable tension from the dual XY servo system. The compound curvature emerges from:
- Base XY setting primary curve through first 4 knuckles
- Tip XY modulating through knuckles 5-8
- Threading position affecting mechanical advantage

**Expected Movement Qualities:**
- CONCORDANT mode: Flowing S-curves
- OPPOSITIONAL mode: Tension where tip works against base
- Compound influence zones with different leverage ratios

## Control System Development Goals

T.W. expressed interest in building an OpenFrameworks control system to:
- Test tentacle positioning systematically
- Understand actual kinematics vs theoretical models
- Manually drive servos and observe mechanical behavior
- Verify influence zones and mechanical limits

**Testing Priorities:**
1. Individual servo effects (Base X only, Base Y only, etc.)
2. Actual influence zones and propagation
3. Mechanical limits and binding conditions
4. Thread position differences on medium knuckles

## Visualization Strategy

**Primary Objective:** Verify movement mechanics match expectations before visualizing spatial coverage patterns

**Implementation Options:**
- OF sending serial commands to Arduino for direct control
- Pure simulation for theoretical kinematics understanding
- Real-time visualization of V31 algorithm execution
- Hybrid approach combining simulation and hardware feedback

**Validation Requirements:**
- Confirm kinematics model accuracy
- Test compound curvature modes
- Verify territorial exploration pattern representation
- Ensure mechanical constraints properly modeled

## Technical Implementation Considerations

**Serial Communication:**
- Parse Arduino debug output for state data
- Handle Base XY and Tip XY position streaming
- Manage energy levels and behavioral state transitions

**3D Rendering:**
- Segment-based representation of tentacle structure
- State-dependent coloring (REST/SEEK/SEARCH/RETURN)
- Real-time position updates
- Interactive control interface

**Kinematic Chain:**
- 8-segment structure with 11mm spacing
- Influence distribution between control zones
- Servo angle to bend angle conversion
- Mechanical constraint enforcement

## Next Development Steps

1. **Basic Control Interface:** Manual servo positioning to understand mechanics
2. **Kinematic Validation:** Compare theoretical vs actual movement
3. **Algorithm Visualization:** Real-time V31 pattern rendering
4. **Spatial Analysis:** Territory coverage and search efficiency metrics

The development emphasizes validation of mechanical understanding before implementing complex behavioral visualization, ensuring accurate representation of the actual cybernetic coupling dynamics.
