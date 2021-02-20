#include "register_core_types.h"
#include "goost/register_types.h"
#include "goost/classes_enabled.gen.h"

#include "core/engine.h"
#include "scene/main/scene_tree.h"

#include "image/register_image_types.h"
#include "math/register_math_types.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_node.h"
#include "editor/editor_resource_preview.h"
#include "types/editor/variant_resource_preview.h"
#endif

namespace goost {

static GoostEngine *_goost = nullptr;

#if defined(TOOLS_ENABLED) && defined(GOOST_VariantResource)
static void _variant_resource_preview_init();
#endif

void register_core_types() {
#ifdef GOOST_GoostEngine
	_goost = memnew(GoostEngine);
	goost::register_class<GoostEngine>();
	Engine::get_singleton()->add_singleton(
			Engine::Singleton("GoostEngine", GoostEngine::get_singleton()));
	SceneTree::add_idle_callback(&GoostEngine::flush_calls);
#endif
	goost::register_class<InvokeState>();

	goost::register_class<Grid2D>();
	goost::register_class<ListNode>();
	goost::register_class<LinkedList>();

	goost::register_class<VariantResource>();
#if defined(TOOLS_ENABLED) && defined(GOOST_VariantResource)
	EditorNode::add_init_callback(_variant_resource_preview_init);
#endif

#ifdef GOOST_IMAGE_ENABLED
	register_image_types();
#endif
#ifdef GOOST_MATH_ENABLED
	register_math_types();
#endif
}

void unregister_core_types() {
	if (_goost) {
		memdelete(_goost);
	}
#ifdef GOOST_IMAGE_ENABLED
	unregister_image_types();
#endif
#ifdef GOOST_MATH_ENABLED
	unregister_math_types();
#endif
}

#if defined(TOOLS_ENABLED) && defined(GOOST_VariantResource)
void _variant_resource_preview_init() {
	Ref<VariantResourcePreviewGenerator> variant_resource_preview;
	variant_resource_preview.instance();
	EditorResourcePreview::get_singleton()->add_preview_generator(variant_resource_preview);
}
#endif

} // namespace goost
