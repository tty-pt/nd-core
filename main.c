#include <nd/nd.h>

struct icon on_icon(unsigned ref __attribute__((unused)),
		unsigned type, unsigned player_ref __attribute__((unused)))
{
        struct icon i = {
                .actions = ACT_LOOK,
                .ch = '?',
                .pi = { .fg = WHITE, .flags = BOLD, },
        };

        switch (type) {
        case TYPE_ROOM:
                i.ch = '-';
                i.pi.fg = YELLOW;
                break;
        case TYPE_ENTITY:
		i.pi.flags = BOLD;
		i.ch = '!';
		i.pi.fg = YELLOW;
                break;
	default: {
	        OBJ what, loc;
		nd_get(HD_OBJ, &what, &what.location);
		nd_get(HD_OBJ, &loc, &what.location);

		if (what.location == player_ref) {
			i.actions |= ACT_DROP;
			break;
		}

		nd_get(HD_OBJ, &loc, &what.location);
		if (loc.type != TYPE_ENTITY)
			i.actions |= ACT_GET;

                break;
        }}

	return i;
}
