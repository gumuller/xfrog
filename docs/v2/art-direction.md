# Xfrog 2.0 art direction

A sunny seaside cartoon town, a determined human pizza courier, and friendly
frog drivers preserve the original game's playful premise. The palette combines
dark teal outlines, coral, butter yellow, cream, turquoise, and fresh greens.
All artwork is full color. The renderer uses RGBA textures (24-bit RGB plus
transparency), a 1920x1080 framebuffer, and linear scaling. Physics and UI share
this coordinate system, so collision zones correspond to the selected artwork.

## Production assets

- `assets/v2/city.png`: the complete seaside town. The selected image has **three**
  horizontal road lanes; the simulation matches those lanes. The original brief
  requested four lanes, but the chosen three-lane result provided clearer gaps.
- `assets/v2/courier.png`: eight walk/run frames in each of four directions,
  32 total. Rows face down, right, up, and left. Frames are cropped at runtime
  inside their own grid cells and aligned at the feet.
- `assets/v2/poses.png`: four directional idle poses, four celebration frames and
  four harmless dizzy reaction frames. The same courier identity was used as an
  image reference.
- `assets/v2/carry-walk.png`: 32 matching walking frames holding a cream pizza
  box with a coral badge in both hands, in the same down/right/up/left row order.
- `assets/v2/carry-poses.png`: four holding idles (down/right/up/left) followed
  by four front-facing handoff frames. Reversed handoff frames animate pickup.
  The authored set contains 84 frames. The original walk sheets are retained
  in production; the newer in-betweens supplement the original wide strides.
- `assets/v2/thief.png`: right/left poses, empty-handed and holding a stolen pizza.
  Standing reference; the running animation now uses `thief-run.png`.
- `assets/v2/traffic.png`: four vehicles in both directions, eight sprites total.
- Lilita One supplies the playful headings. Atkinson Hyperlegible supplies clear
  small text. Their SIL Open Font Licenses are included beside the fonts.
- Shadows, destination rings, particles, trails, buttons, and HUD shapes are
  rendered in code, using the same palette.

Artwork was generated using the built-in OpenAI image generation tool, then
copied into the repository. PNG transparency is preserved; no online image
service is used at runtime. The following are the production prompt set.

## City prompt

Use case: stylized-concept. Production-ready background illustration for a REAL
playable 2D cartoon pizza-delivery arcade game called Xfrog, not a mockup.
Landscape 16:9, ideally 1920x1080. Fixed overhead/three-quarter arcade view,
coherent miniature European seaside city, sunny warm late afternoon. Painterly
clean polished commercial indie-game art, rounded chunky buildings, charming
details, gentle material texture, bold readable silhouettes, consistent dark teal
outlines and soft ambient shading, palette warm butter cream, coral terracotta,
teal turquoise, fresh green. COMPOSITION CRITICAL: screen top 0-11% mostly softly
textured dark teal sky/flat background reserved for separate HUD; row of FIVE small
charming distinct delivery storefronts/buildings across x10%-90% between y12%-29%,
with their front doors facing downward, sidewalk y29%-34%. Four perfectly STRAIGHT
HORIZONTAL unbroken open asphalt road lanes occupy y35%-73%, with evenly spaced
lanes centered y40%,49%,58%,67%. Empty road, no vehicles or pedestrians anywhere;
very dark blue-grey asphalt with subtle texture, cream dashed lines. Bottom
y74%-100% is broad safe pedestrian plaza, pale warm stone paving and sparse plants
at left/right corners. A charming coral-roof pizza shop sits at bottom center,
its doorway at x50% y88%, with a green/cream striped awning, warm illuminated
windows and small pizza emblem. Keep the central plaza above the shop empty for
player movement. Top storefronts are pastel bakery, green grocer, hotel, workshop
and cafe. Buildings can have little blank signs but NO WORDS, NO LETTERS, NO TEXT,
no UI or score, no logos. Do not add characters, traffic, objects in the roads or
dividers blocking movement. Camera is locked top-front arcade perspective, no
horizon point, entire image consistently orthographic-ish and readable as
collision-aligned game art. This is the final background art to be used directly
in an SDL game.

## Courier prompt

Use case: stylized-concept. Production sprite atlas for real 2D arcade
pizza-courier game Xfrog 2.0. EXACT regular grid EIGHT columns by FOUR rows,
32 full-body individual sprites, evenly centered in equal-sized invisible grid
cells, each sprite fully contained with generous transparent gutters. Landscape
atlas. Every sprite depicts THE SAME charming young human pizza courier: big
expressive friendly face, curly dark hair peeking under small coral-red cap,
teal turquoise short-sleeve jacket, cream shirt, dark navy trousers, coral red
sneakers, small squared tan pizza-delivery backpack with a tiny round pizza badge.
Charming hand-painted commercial cartoon game art, clean dark teal outlines,
soft warm directional shading from upper left, chunky readable silhouette and
body proportions consistent in ALL 32 frames. View is elevated three-quarter
orthographic arcade camera looking down about 30 degrees, NOT flat profile or
realistic. Row 1 is eight consecutive frames of a seamless running/walking cycle
facing DOWN toward the camera, alternating feet and arms with energetic bounce.
Row 2 same eight consecutive cycle frames facing RIGHT. Row 3 eight frames facing
UP away from camera, pizza backpack visible. Row 4 eight frames facing LEFT.
Character identical size, same cap/clothes/backpack palette throughout. Face and
body coherent, no extra limbs. Place foot baseline and center exactly consistently
in every grid cell. Sprites must be SEPARATED, no overlap, no environment, no
background, no ground plane, no shadows, no labels, no text, no grid lines. Actual
transparent alpha background. This is final game-ready production atlas artwork.

## Traffic prompt

Use case: stylized-concept. Final production game sprite atlas for Xfrog 2.0,
cheerful hand-painted cartoon arcade pizza-delivery game. EXACT even regular grid
FOUR columns and TWO rows, 8 separated vehicles on true transparent alpha
background. Same elevated three-quarter side view as a 2D arcade game, camera
looking down 30 degrees with visible front SIDE and roof. All vehicles travel
horizontally across the screen, long axes perfectly horizontal, NOT
diagonal/isometric. Row 1: four vehicles facing RIGHT: a butter-yellow rounded
tiny hatchback with cream roof, a teal turquoise rounded compact car, a coral-red
charming small delivery van, a powder-blue/cream retro mini bus. Row 2: the exact
same four vehicles facing LEFT in same order. Same size/style/color for the
matching right/left pair; slight difference in cute driver pose welcome. Each has
small friendly green cartoon frog driver visible in side window, no human drivers.
Style clean polished commercial indie-game illustration, chunky shapes, confident
dark teal outlines, soft warm lighting from upper left, subtle material shading,
warm cream reflections, rounded big dark tires, FUN and friendly and readable.
Match palette coral/teal/butter-yellow/cream, no photorealism, no pixel art.
Vehicles centered horizontally and vertically in eight EQUAL INVISIBLE cells,
ample transparent margins, no cropping or overlapping, no wheels outside cell.
No labels, text, words, logos, scenery, backgrounds, ground or cast shadows.
Genuine transparency. Landscape atlas. This atlas will be directly cut into
eight game sprites.

## Idle and reaction atlas prompt

Reference image: `assets/v2/courier.png` (character identity and visual style).

Use case: identity-preserve / stylized-concept. The reference image is the final
walking-sprite atlas for the human courier in Xfrog 2.0. Create a NEW complementary
production atlas of the EXACT SAME character, identical face, childlike proportions,
curly dark hair, coral red cap, teal jacket, cream shirt, dark navy trousers, coral
sneakers, tan pizza backpack with round pizza badge, same dark teal outlines and
warm shaded cartoon illustration style. Do not change character identity or design.
Camera is identical elevated three-quarter 2D arcade camera. EXACT regular grid
FOUR columns by THREE rows, 12 separated full-body sprites, all equal-size invisible
cells, each character centered with consistent foot baseline and no overlap.
Row 1: four NEUTRAL STANDING IDLE poses, BOTH feet firmly resting on the same
baseline, relaxed arms and friendly expression; face DOWN toward camera, RIGHT,
UP away, LEFT in that order. These are idle standing frames, no running or raised
feet. Row 2: four sequential CELEBRATION frames facing camera, smiling and making
a small happy thumbs-up / wave after delivering a pizza. Keep feet grounded
throughout, small natural arm changes, not huge jumps. Row 3: four sequential
funny harmless DIZZY/OUCH reaction frames facing camera, with a sheepish smile
and gentle rubbing of head / wobbling arms, no injuries, no floating stars or
extra objects; standing body stays consistent. Preserve exact outfit, palette,
thickness of outlines, head size, lighting, full body height across all twelve
frames. Production clean isolated sprites with genuine transparent alpha
background, generous transparent gutters. No floor, shadows, scenery, labels,
words, text, grid lines or watermarks. This is a NEW atlas to complement the
existing 32-frame walking atlas, not a replacement for it.

## Pizza-carrying atlas production prompts

Built-in image_gen was used with transparent backgrounds and the existing
courier/pose atlases as visual identity references. Final selected PNGs are
`assets/v2/carry-walk.png` and `assets/v2/carry-poses.png`; their original alpha
is preserved. Discarded drafts are not shipped.

Carry walk spec: identity-preserve / stylized-concept. Match the existing courier
exactly: coral cap, curly dark hair, teal jacket, cream shirt, navy trousers,
coral sneakers, tan backpack, warm cartoon shading and dark teal outlines.
Produce an exact 8-column by 4-row transparent atlas, 32 complete walking frames.
Rows face down, right, up and left. Every frame holds the same closed cream
pizza box with coral emblem horizontally in both hands; box remains visible
in the back-facing row beyond the shoulders. Consistent body scale and foot
baseline, generous gutters, full body in every cell, no backgrounds or labels.

Carry pose spec: identity-preserve. Match that character and box exactly in an
exact 4-column by 2-row transparent atlas. Top row: four grounded standing poses
holding the box, facing down, right, up away from camera, left. Bottom row:
four front-facing handoff frames: box at chest, arms extend halfway, box fully
offered, then empty hands after delivery. Preserve consistent silhouette, outfit,
lighting and baseline. Full bodies, no cropping, shadows, text or grid lines.

Targeted correction: change only the third sprite in the top row to face UP
away from camera, with backpack visible and pizza box held beyond the shoulders.
Preserve the other seven sprites and transparency unchanged.

## Returning thief production prompt

Built-in image_gen, transparent background. Final saved asset:
`assets/v2/thief.png` (2 columns by 2 rows). Use case: stylized-concept.
A lanky, mischievous friendly human pizza thief, purple flat cap, black eye mask,
cream-and-black striped shirt, dark plum trousers, brown rounded shoes,
expressive sly smile and big nose. Match the town's warm shaded cartoon art,
dark teal outlines and elevated three-quarter camera. Four full-body grounded
standing poses: top row right/left empty-handed; bottom row right/left holding
a closed cream pizza box with coral emblem in both hands. Identical body scale
and outfit across cells, generous transparent margins, no shadows, scenery,
text, grid lines or floating marks. The original generated alpha is preserved.

## Runtime cleanup and animation

Disconnected alpha marks are rejected. Running sheets are extracted as whole
connected sprites, preserving broad strides even if feet extend past nominal
cell boundaries. See the current wide-stride restoration below.

## Wide-stride running restoration

The original broad running poses are used again. New assets, generated with the
built-in image tool and preserved as RGBA PNGs:

- `assets/v2/run-inbetweens.png`: 4x4 missing courier recoil/passing poses.
- `assets/v2/carry-inbetweens.png`: the same 4x4 poses holding the pizza box.
- `assets/v2/thief-run.png`: 4x4 atlas; first eight run poses empty-handed,
  next eight holding stolen pizza. All face right and are mirrored for left.

Courier prompt: preserve exact reference character, coral cap, teal jacket,
cream shirt, navy trousers, coral shoes and tan backpack. Four columns by four
rows, directions down/right/up/left. Columns are left-leg recoil, left-support
passing, right-leg recoil, right-support passing. Bent knees close under hips
and supporting foot under body; these are the missing in-betweens between the
existing wide strides. Same head/body scale, full bodies, generous transparent
margins, no text, marks or shadows.

Carrying edit prompt: preserve that exact 4x4 atlas, all poses, legs, identity and
transparency; add the closed cream pizza box with coral emblem held in both
hands, matching the original carry atlas. Back view box visible beyond shoulders.

Thief prompt: preserve the purple cap, eye mask, striped shirt, plum trousers,
brown shoes and face. Eight chronological running poses alternating wide open
leg contacts, down/recoil, bent passing legs, knee drive, then opposite-leg
versions. Empty-handed followed by pizza-carrying. Relayout to four columns and
four rows, same scale, full shoes and limbs in every sprite, transparent gutters,
no labels or shadows.

Whole connected-body extraction replaces nominal grid cuts for running sheets,
so the widest feet remain intact. Rows share a reference scale and heads are
registered horizontally. The courier plays twelve chronological key drawings
per direction; the thief plays eight per carrying state. No small-stride mesh
animation is used for running.

## Current courier playback correction

The courier uses only `courier.png` and `carry-walk.png` when running. The
in-between sheets above are retained as earlier source variants and are not
loaded. The side-view contact and recoil slots are reordered, with one coherent
sheet and orientation per cycle. Left travel mirrors the whole right cycle;
there is no front/back mid-cycle mirroring. Idle/reaction art is unchanged.
The thief still uses only `thief-run.png`, now rendered at 110 pixels high.
No artwork files changed in this correction. Discarded generation experiments
were not copied into the repository or used in the game.
