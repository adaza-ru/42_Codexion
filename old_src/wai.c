
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

static int	can_take_dongles(t_coder *coder, int left, int right)
{
	t_env	*env;
	int		i;
	int		other_id;
	int		ahead;

	env = coder->env;
	if (!dongles_available(env, left, right))
		return (0);
	i = 0;
	ahead = 0;
	while (i < env->heap_size)
	{
		other_id = env->heap[i];
		if ((other_id == left || other_id == right
				|| (other_id + 1) % env->num_coders == left
				|| (other_id + 1) % env->num_coders == right)
			&& has_higher_priority(env, other_id, coder->id))
		{
			if (env->scheduler == SCH_FIFO || !is_doomed(coder, ahead))
				return (0);
			ahead++;
		}
		i++;
	}
	return (1);
}

static void	wait_on_signal(t_coder *coder, int left, int right)
{
	t_env			*env;
	struct timespec	ts;
	size_t			deadline;

	env = coder->env;
	deadline = env->dongle_free_at[left];
	if (env->dongle_free_at[right] > deadline)
		deadline = env->dongle_free_at[right];
	if (deadline <= get_current_time())
	{
		pthread_cond_wait(&env->cond_coders[coder->id],
			&env->arbitrator_mutex);
		return ;
	}
	ts.tv_sec = deadline / 1000;
	ts.tv_nsec = (deadline % 1000) * 1000000;
	pthread_cond_timedwait(&env->cond_coders[coder->id],
		&env->arbitrator_mutex, &ts);
}

int	wait_for_turn(t_coder *coder)
{
	t_env	*env;
	int		left;
	int		right;

	env = coder->env;
	left = coder->id;
	right = (coder->id + 1) % env->num_coders;
	pthread_mutex_lock(&env->arbitrator_mutex);
	enqueue_coder(env, coder);
	while (!can_take_dongles(coder, left, right) && !check_simulation_end(env))
		wait_on_signal(coder, left, right);
	dequeue_coder(env, coder);
	if (check_simulation_end(env))
	{
		pthread_mutex_unlock(&env->arbitrator_mutex);
		return (0);
	}
	env->dongle_taken[left] = 1;
	env->dongle_taken[right] = 1;
	pthread_mutex_unlock(&env->arbitrator_mutex);
	return (1);
}