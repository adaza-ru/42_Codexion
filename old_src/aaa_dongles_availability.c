
#include "h_codexion.h"

static int	is_doomed(t_coder *coder, int ahead)
{
	t_env	*env;
	size_t	time_lived;
	size_t	time_left;
	size_t	time_needed;
	size_t	margin;

	env = coder->env;
	time_lived = get_current_time() - coder->last_compile_start;
	if (time_lived >= env->time_to_burnout)
		return (1);
	time_left = env->time_to_burnout - time_lived;
	margin = env->time_to_burnout / 10;
	if (margin > 15)
		margin = 15;
	if (time_left <= margin)
		return (1);
	time_needed = (ahead + 1) * (env->time_to_compile + env->dongle_cooldown);
	if (time_needed >= (time_left - margin))
		return (1);
	return (0);
}

static int	dongles_available(t_env *env, int left, int right)
{
	size_t	now;

	if (env->dongle_taken[left] != 0 || env->dongle_taken[right] != 0)
		return (0);
	now = get_current_time();
	if (now < env->dongle_free_at[left] || now < env->dongle_free_at[right])
		return (0);
	return (1);
}

static int	is_conflicting_id(t_env *env, int other_id, int left, int right)
{
	if (other_id == left || other_id == right)
		return (1);
	if ((other_id + 1) % env->num_coders == left)
		return (1);
	return ((other_id + 1) % env->num_coders == right);
}

int	can_take_dongles(t_coder *coder)
{
	t_env	*env;
	int		left;
	int		right;
	int		i;
	int		other_id;

	env = coder->env;
	left = coder->id;
	right = (coder->id + 1) % env->num_coders;
	if (!dongles_available(env, left, right))
		return (0);
	i = 0;
	while (i < env->heap_size)
	{
		other_id = env->heap[i];
		if (other_id == coder->id)
			break ;
		if (is_conflicting_id(env, other_id, left, right))
			if (env->scheduler == SCH_FIFO || !is_doomed(coder, i))
				return (0);
		i++;
	}
	return (1);
}
