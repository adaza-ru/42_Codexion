/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 02:30:11 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/09/08 20:17:25 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

static void	wait_on_signal(t_env *env, t_coder *coder, int left, int right)
{
	struct timespec	ts;
	size_t			deadline;

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

static int	wait_for_turn(t_env *env, t_coder *coder)
{
	int				left;
	int				right;

	left = coder->id;
	right = (coder->id + 1) % env->num_coders;
	pthread_mutex_lock(&env->arbitrator_mutex);
	enqueue_coder(env, coder);
	while (!can_take_dongles(env, coder, left, right)
		&& !check_simulation_end(env))
		wait_on_signal(env, coder, left, right);
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

int	take_dongles(t_coder *coder)
{
	if (!wait_for_turn(coder->env, coder))
		return (0);
	print_status(coder, DONGLE_CLR, "has taken a dongle");
	print_status(coder, DONGLE_CLR, "has taken a dongle");
	return (1);
}

void	release_dongles(t_coder *coder)
{
	t_env	*env;
	int		left_neighbor;
	int		right_neighbor;
	size_t	free_at;

	env = coder->env;
	pthread_mutex_lock(&env->arbitrator_mutex);
	free_at = get_current_time() + env->dongle_cooldown;
	env->dongle_taken[coder->id] = 0;
	env->dongle_taken[(coder->id + 1) % env->num_coders] = 0;
	env->dongle_free_at[coder->id] = free_at;
	env->dongle_free_at[(coder->id + 1) % env->num_coders] = free_at;
	left_neighbor = (coder->id - 1 + env->num_coders) % env->num_coders;
	right_neighbor = (coder->id + 1) % env->num_coders;
	pthread_cond_broadcast(&env->cond_coders[left_neighbor]);
	pthread_cond_broadcast(&env->cond_coders[right_neighbor]);
	pthread_mutex_unlock(&env->arbitrator_mutex);
}
