# The Pizza Run

## The playable loop

Start at the pizza shop, press Space to take one fresh order, and follow the
marked storefront. Walk around the physical shop, read the three traffic lanes,
and cross when a gap opens. Deliver with Space, earn a heat-dependent tip, and
return for the next order. Finish six deliveries to clear the first shift;
subsequent shifts increase the delivery goal and traffic speed.

The movement simulation runs at a fixed 120 Hz. Acceleration is responsive,
render positions interpolate between simulation ticks, diagonal speed is normalized, and the short dash has a visible cooldown. Dash
is a movement tool, not invulnerability. A collision drops the pizza, breaks the
streak, costs one heart, and returns the courier to safety with a grace period.
Collision uses feet/bumper bounds instead of the painted roofs of the vehicles.

Hot pizzas earn more, consecutive deliveries add a streak tip, and close calls
add a small bonus once per passing car. Cold pizzas expire without costing a
heart. Relaxed mode has five hearts, a three-minute shift and fewer cars. Arcade
has three hearts, a two-and-a-half-minute shift and denser traffic.

## Presentation and accessibility

The map, characters, traffic, typography and effects share one palette. The HUD
keeps the objective, order destination, heat, time, score, hearts and dash charge
visible. Help and pause freeze simulation and sound; losing window focus pauses
automatically. Mouse buttons, arrows/WASD, and common gamepad actions are supported.
Music is an original 80-bar, 104 BPM, 184.6-second stereo arrangement with
sample-style flute, piano, vibraphone, strings, bass and soft percussion. The
intro, A/B themes, quiet bridge and reprise avoid a short repetitive loop.
`compose_theme.py` renders the included 32 kHz PCM score; there are no external
recordings or melodies. Nine effects include a cash-drawer clack and bell on
delivery, and a distinct theft cue. There is a
persistent sound toggle. Records, delivery totals and settings use a separate
per-user profile, with atomic writes and backup recovery.

## Improvements included

- Forgiving collision bounds and respawn grace.
- A relaxed mode alongside a more demanding arcade mode.
- Dash, heat-dependent tips, delivery streaks, close-call bonuses and successive
  shifts give the player choices beyond merely crossing a road.
- Clear destination rings, contextual interaction prompts, feedback particles,
  sound cues, and an always-visible objective.
- A complete menu, help, pause, results, restart and next-shift flow.

## Suggested next additions

1. Additional districts with their own routes and traffic patterns: market,
   waterfront, and rainy evening streets would add more variety than increasing
   speed alone.
2. Courier outfits and backpack designs unlocked by delivery milestones, while
   retaining the same collision silhouette and readable animation.
3. Seeded daily routes and local medals, with separate relaxed/arcade records.
4. Optional traffic events such as a pedestrian crossing and an ice-cream van,
   with clear advance warnings and playtests for fairness.

These are future extensions, not features claimed by this build. The preserved
1.03 implementation remains available as an optional classic target and at its
published Git tag; the HD game is the primary 2.0 implementation on both platforms.

Courier running uses three directional atlases of separate painted body parts.
A continuous two-bone leg rig follows contact, compression, push-off, recovery
and landing poses; the opposite leg is half a cycle ahead. The stance foot moves
backward at the same speed as player travel, with an airborne interval between
steps. Authored Hermite tangents match across the loop boundary. The foot path
stays away from fully folded and fully straight knees to avoid IK popping.
Arms counter the legs; the carrying torso keeps both hands on the pizza box.
Left travel mirrors the whole right rig. Thief artwork is never used for it.

The model clock stays in [0, 1), with explicit wrap-aware render interpolation.
Gait rate is speed / (88 * 1.4), capped at 3.5 cycles/second during dash. Standing
uses the same parts, blending to a neutral stance below 160 pixels/second.
Tests cover fixed bone lengths, reach limits, planted feet, alternating contacts,
joint velocity at every key and the seam, 30/60/120 FPS, and 96 rendered samples
in all four directions with/without pizza. Model-ticked playback exercises
repeated loops at normal speed. Visual review is still required alongside tests.

The pose basis follows [Adobe's run-cycle guide](https://www.adobe.com/uk/creativecloud/animation/discover/animation-run-cycle.html);
limb constraints follow [Spine's two-bone IK guidance](https://esotericsoftware.com/spine-ik-constraints).
The earlier optical-flow interpolation was removed because matching pixels
between drawings did not establish a consistent anatomical sequence.

Both gameplay characters use the same 88-pixel height, a 20% reduction from 110.
The thief keeps its original eight-pose animation and sidewalk behavior. Courier
idle, pickup, handoff, hit reaction, celebration, shadows and dash trails use the
reduced size too. The menu illustration retains its independent layout size.

## Returning challenge

Traffic level is `min(8, shift + delivered / 2 + elapsed / 50)` (integer steps).
Each step within a shift raises base traffic speed by 4.5%. Relaxed mode keeps
its five hearts and generous timer while sharing the same progression.
All traffic has a consistent 170-pixel footprint; rendered width is 196 pixels
with height capped at 146, so vans and buses remain visually comparable to cars.

At level 3, an eligible outer-lane car can request a lane change. The opposing
middle lane is crossed only after checking projected positions throughout the
3.2-second warning/maneuver interval. Changes are never initiated within 260
horizontal pixels of the courier. Amber turn signals and a ground path warn for
1.2 seconds; smoothstep motion takes two seconds. Only one car changes at once. A faster car can request overtaking at any traffic
level if a slower leader is ahead and the path is safe. Blocked cars brake and
match their leaders. All movement is solved from the same positions, then queue
constraints preserve at least 214 pixels between equal-sized car centers.
Screen wrapping preserves movement overshoot rather than resetting spacing.

Merge validation solves the exact lateral overlap interval of the smoothstep
path and checks the entire relative-motion sweep on the periodic road. It is
not based on sparse sampled points. A planned speed is held through the signal
and merge; the path is checked again when the signal ends. Tests cover blocked
and successful overtaking, queues in both directions, repeated road wraps and
800 seconds of later-shift traffic across eight seeds.

The thief warns for 1.5 seconds, then runs along the delivery sidewalk at y=350.
Every appearance uses that upper sidewalk, next to the storefront addresses.
He alternates horizontal direction between appearances. Contact steals a carried pizza,
breaks the streak and plays a cue without costing a heart. His carrying sprite
shows the stolen order; he escapes faster and cannot steal again that pass.
Pause/help freeze both thief and traffic. Later levels shorten his cooldown,
with at least nine seconds between passes.

Pickup and delivery prompts use scalable cream speech balloons, dark outlines,
soft shadows, tails and Space key badges. They anchor above the interaction
without covering the courier's head.
