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
  image reference, bringing the complete character set to 44 frames.
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
