# The Pizza Run

## The playable loop

Start at the pizza shop, press Space to take one fresh order, and follow the
marked storefront. Walk around the physical shop, read the three traffic lanes,
and cross when a gap opens. Deliver with Space, earn a heat-dependent tip, and
return for the next order. Finish six deliveries to clear the first shift;
subsequent shifts increase the delivery goal and traffic speed.

The movement simulation runs at a fixed 120 Hz. Acceleration is responsive,
diagonal speed is normalized, and the short dash has a visible cooldown. Dash
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
Music and sound effects are an original synthesized score and cues, with a
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
