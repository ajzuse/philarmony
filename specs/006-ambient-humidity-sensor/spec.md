# Feature Specification: Ambient Humidity Sensor Support

**Feature Branch**: `006-ambient-humidity-sensor`

**Created**: 2026-08-19

**Status**: Draft

**Input**: User description: "quero criar uma nova tarefa spec antes da spec de testes, que de suporte a um segundo sensor de temperatura e umidade no mesmo estilo do interno, com a mesma compatibilidade de sensores, mas para a temperatura e umidade ambiente. Isso faz com que o controle no esp32 quando liga o fan para baixar a umidade da cabine, leve a umidade do ambiente em consideração para saber quando desligar o fan. Não é obrigatório ter 2 sensores, mas caso possua é desejado que use para saber quando parar de ventilar."

**Roadmap position**: Product feature that should be planned/implemented **before** concentrating remaining work on `005-manual-validation` (manual validation catalog). Numbering is sequential (`006`); priority relative to open manual validation work is higher.

## Feature Overview

**Feature Name**: Ambient Temperature & Humidity Sensor  
**Short Name**: ambient-humidity-sensor  

Add optional support for a second temperature/humidity sensor that measures **ambient** (outside the drying chamber) conditions, using the **same supported sensor catalog and configuration style** as chamber sensors. Chamber and ambient **MAY use different models** from that catalog (they are not required to match each other). When the ambient sensor is present and healthy, exhaust-fan control that aims to lower chamber humidity MUST use ambient humidity to decide when continued ventilation is no longer useful and the fan can stop. A single chamber sensor remains a fully valid configuration; ambient sensing is optional enhancement, not a requirement.

## Clarifications

### Session 2026-08-19

- Q: Must the ambient sensor be the same hardware model as the chamber sensor? → A: No — both draw from the same supported sensor catalog, but each may be a different model (e.g. chamber DHT22 + ambient SHT31). Matching models is allowed, never required.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Optional Ambient Sensor Improves Fan Stop Decision (Priority: P1)

A user builds a dryer with both a chamber sensor and an ambient sensor. During a drying cycle, when the system runs the exhaust fan to reduce chamber humidity, the controller compares chamber humidity to ambient humidity. Once chamber humidity is no longer meaningfully higher than ambient (ventilation can no longer dry the air), the fan stops instead of running uselessly or pulling in equally humid (or wetter) air.

**Why this priority**: This is the core value of the feature — smarter ventilation that respects physical limits of ambient air.

**Independent Test**: Configure chamber + ambient sensors with chamber humidity above target and ambient humidity close to or above chamber humidity; start a cycle that would otherwise keep exhausting; verify the fan stops (or does not keep exhausting) based on ambient comparison while heater/cycle rules remain consistent with base firmware.

**Acceptance Scenarios**:

1. **Given** chamber and ambient sensors are configured and healthy, chamber humidity is above the cycle target, and ambient humidity is lower than chamber humidity by more than the useless-ventilation margin, **When** the cycle is drying and exhaust is used to lower humidity, **Then** the fan may run while ventilation remains useful.
2. **Given** the same dual-sensor setup, **When** chamber humidity falls to within the useless-ventilation margin of ambient humidity (or ambient is equal/higher), **Then** the controller stops the humidity-reduction exhaust fan rather than continuing ineffective ventilation.
3. **Given** dual sensors and the fan stopped due to ambient-limited humidity, **When** chamber humidity later rises again above ambient by more than the margin (e.g. moisture released from filament), **Then** the controller may resume exhaust for humidity reduction if the cycle is still active and other stop conditions have not been met.

---

### User Story 2 - Single Chamber Sensor Remains Fully Supported (Priority: P1)

A user with only the chamber temperature/humidity sensor continues to operate exactly as today: cycles start/stop, target humidity and max time still apply, and fan cooldown / safety behavior from the base firmware is unchanged. Ambient sensing is never required.

**Why this priority**: Optional hardware must not break the common single-sensor build.

**Independent Test**: Configure only the chamber sensor (no ambient); run a normal drying cycle with target humidity; verify fan and cycle completion match existing base-firmware behavior.

**Acceptance Scenarios**:

1. **Given** only a chamber sensor is configured (ambient absent or explicitly disabled), **When** the user runs a drying cycle, **Then** humidity and fan behavior follow the existing chamber-only rules (target humidity, max time, safety, post-completion cooldown).
2. **Given** a device that previously ran with one sensor, **When** firmware/config supporting ambient is installed but ambient is not configured, **Then** the device remains operable without requiring a second sensor.

---

### User Story 3 - Configure Ambient Sensor From the Shared Catalog (Priority: P2)

A user (via installer, control app, or equivalent configuration path) adds an ambient temperature/humidity sensor by picking any type from the project’s supported temperature/humidity catalog and configuring it in the same style as other sensors (type, bus/pins, calibration), distinguished by role/placement as ambient rather than chamber. The ambient model need not match the chamber model.

**Why this priority**: Hardware flexibility and parity with the existing sensor matrix are required for DIY builds, but secondary to correct control behavior.

**Independent Test**: Configure chamber and ambient with two different supported types (or the same type); save and reload config; verify both are accepted, ambient readings appear in status, and fan decisions use ambient when present.

**Acceptance Scenarios**:

1. **Given** the device supports the project’s temperature/humidity sensor catalog, **When** the user configures an ambient sensor of any supported type (including a type different from the chamber sensor), **Then** the configuration is accepted and persisted like other sensors.
2. **Given** chamber uses model A and ambient uses model B (both supported), **When** the device runs, **Then** both sensors operate independently and ambient-aware fan logic still applies when ambient humidity is valid.
3. **Given** an ambient sensor is configured, **When** status is streamed during operation, **Then** ambient temperature and humidity are available alongside chamber readings (or clearly labeled equivalents).
4. **Given** the user selects an unsupported sensor type for ambient, **When** they save configuration, **Then** the system rejects the choice with a clear error (same class of validation as chamber sensors).

---

### User Story 4 - Ambient Sensor Fault Falls Back Safely (Priority: P2)

If the ambient sensor fails, disconnects, or returns invalid readings while a cycle is running, the system MUST NOT treat ambient as required hardware: it falls back to chamber-only fan/humidity behavior and surfaces a non-fatal ambient sensor problem to monitoring surfaces.

**Why this priority**: Optional sensor must fail soft; chamber safety remains governed by existing safety rules.

**Independent Test**: Start a dual-sensor cycle, then disconnect or invalidate the ambient sensor; verify fallback to chamber-only humidity/fan logic and that chamber safety cutoffs still work.

**Acceptance Scenarios**:

1. **Given** dual sensors and an active cycle, **When** ambient readings become invalid beyond the configured sensor timeout, **Then** ambient-aware fan stopping is disabled and chamber-only rules apply.
2. **Given** ambient has failed, **When** operators view status, **Then** they can tell that ambient data is unavailable/error without the whole dryer being forced into a fault solely because ambient is missing.
3. **Given** chamber sensor fails, **When** safety rules of the base firmware apply, **Then** heater/fan failsafe behavior is unchanged by the presence of an ambient sensor.

---

### Edge Cases

- Ambient humidity higher than chamber humidity at cycle start: humidity-reduction exhaust should not run (or should stop immediately); drying may still rely on heat/time per existing cycle rules.
- Ambient and chamber humidity nearly equal (within margin): treat ventilation as useless for humidity reduction.
- Rapid ambient swings (door open, AC on/off): decisions use recent valid readings with the same validation class as other sensors (timeout, range); no oscillation that chatters the fan faster than existing actuator constraints allow.
- Ambient temperature present but humidity capability missing (misconfigured sensor): treat ambient humidity as unavailable → chamber-only fan logic.
- Two integrated sensors on the same bus (e.g. two I2C devices), including two of the same model or two different catalog models: configuration must allow distinct addresses/pins like other multi-sensor setups.
- Chamber and ambient configured as different catalog models: fully valid; no requirement to pair identical hardware.
- Post-completion exhaust cooldown from base firmware: ambient-aware “stop ventilating for humidity” does not cancel mandatory safety/cooldown exhaust required after heater-off completion unless product safety rules already allow cancel; cooldown remains a separate concern from humidity-reduction exhaust.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: System MUST allow an optional ambient temperature/humidity sensor in addition to the chamber sensor, drawn from the same supported sensor catalog and configuration style as chamber sensors.
- **FR-001a**: System MUST allow chamber and ambient to use different models from that catalog; identical models are permitted but MUST NOT be required.
- **FR-002**: System MUST NOT require an ambient sensor for normal operation; chamber-only configurations MUST remain fully supported.
- **FR-003**: When an ambient humidity reading is available and valid, and the exhaust fan is being used to reduce chamber humidity, the controller MUST consider ambient humidity when deciding to stop (or not continue) that ventilation.
- **FR-004**: Ventilation for humidity reduction MUST be treated as no longer useful when chamber humidity is less than or equal to ambient humidity, or within a small configured useless-ventilation margin of ambient humidity (default margin: 2 percentage points RH), and the fan used for that purpose MUST stop under those conditions.
- **FR-005**: When ambient humidity is unavailable (not configured, disabled, or invalid), humidity and fan behavior MUST follow existing chamber-only rules from the base firmware (target humidity, max time, safety, completion cooldown).
- **FR-006**: Status presented to operators MUST expose ambient temperature and humidity when the ambient sensor is configured, clearly distinguishable from chamber readings.
- **FR-007**: Ambient sensor faults MUST fall back to chamber-only humidity-reduction logic without forcing a device-wide fault solely due to ambient failure; chamber sensor faults continue to follow existing safety rules.
- **FR-008**: Configuration paths that today set chamber sensors (installer / control surfaces as applicable) MUST be able to set or omit the ambient sensor without breaking existing chamber configuration.
- **FR-009**: Ambient support MUST reuse the project’s existing sensor compatibility set for temperature/humidity (same catalog accepted for chamber use); no ambient-only exotic sensor requirement, and no rule that ambient model equals chamber model.
- **FR-010**: Cycle completion rules based on target chamber humidity and max time remain authoritative for ending the drying cycle; ambient-aware fan control adjusts ventilation usefulness and MUST NOT invent a new cycle target based on ambient humidity alone.

### Key Entities

- **Chamber Sensor**: Existing temperature/humidity measurement inside the drying chamber; primary input for cycle control and safety.
- **Ambient Sensor**: Optional temperature/humidity measurement of room/outside air; same configuration shape and shared catalog as chamber sensors, distinct role/placement (`ambient`); model MAY differ from the chamber sensor.
- **Supported Sensor Catalog**: The project’s set of accepted temperature/humidity sensor types/families for DIY builds; both chamber and ambient pick independently from this set.
- **Useless-Ventilation Margin**: Small humidity gap (default 2 %RH) below which chamber vs ambient difference is considered insufficient for useful exhaust drying.
- **Humidity-Reduction Exhaust**: Exhaust fan activity intended to lower chamber humidity during a cycle (distinct from mandatory post-heater safety/completion cooldown when those rules apply).
- **Drying Cycle**: Unchanged core entity (targets, duration, stop reasons); gains optional ambient-informed ventilation behavior when ambient is present.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: With dual sensors and chamber humidity within 2 %RH of ambient (or ambient ≥ chamber), humidity-reduction exhaust stops within 5 seconds of that condition being stably met.
- **SC-002**: Chamber-only devices complete the same drying journeys as before ambient support, with no new mandatory configuration steps for a second sensor.
- **SC-003**: When ambient is configured, operators can identify ambient temperature and humidity in the live status view within one status update interval of a valid reading.
- **SC-004**: After ambient sensor loss during a cycle, the system returns to chamber-only fan/humidity decisions within one sensor-timeout interval and continues the cycle unless chamber safety rules require abort.
- **SC-005**: At least 90% of DIY builders who add any second sensor from the supported catalog (same or different model than the chamber sensor) can enable ambient-aware fan stopping without being forced to match the chamber hardware model.
- **SC-006**: Ambient-aware stopping does not prevent mandatory safety cutoffs: heater still cuts on chamber over-temp / chamber sensor safety faults under existing limits.

## Assumptions

- “Same style / same compatibility” means the same configuration model and **shared driver catalog** as chamber temperature/humidity sensors (integrated temp+humidity or equivalent capable sensors), distinguished by role/placement — **not** a requirement that ambient and chamber be the same physical model or SKU.
- Default useless-ventilation margin is **2 percentage points RH**; builders may tighten/loosen later via configuration if exposed, but v1 may ship the default without a user-facing advanced control.
- Ambient sensor is optional; absence is the normal case for minimal builds.
- Ambient-aware logic applies to **humidity-reduction ventilation during drying**, not to replacing target chamber humidity as the cycle success criterion.
- Post-completion / safety exhaust cooldown from the base firmware remains in force; this feature does not remove required cooldown behavior.
- Installer and control app surfaces should eventually expose ambient configuration and readings; firmware behavior is the source of truth for fan decisions.
- Roadmap: this product feature is intended to progress **before** remaining focus on `005-manual-validation`; open manual smoke for this feature, when needed, is catalogued under `005` per constitution, while automated flow tests live on this product spec.
- Single drying chamber / single ambient reference (one ambient zone), not multi-room sensing.

**Constitution compliance:** PROJECT_NAME = Philarmony, all constitutional provisions implemented as per `.specify/memory/constitution.md`
