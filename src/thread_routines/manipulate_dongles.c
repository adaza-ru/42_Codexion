/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   manipulate_dongles.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 02:30:11 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/09/06 02:25:30 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

static size_t	next_wake_deadline(t_env *env, int left, int right)
{
	size_t	deadline;

	deadline = env->dongle_free_at[left];
	if (env->dongle_free_at[right] > deadline)
		deadline = env->dongle_free_at[right];
	return (deadline);
}

static int	wait_in_queue(t_coder *coder, int left, int right)
{
	t_env			*env;
	struct timespec	ts;
	size_t			deadline;

	env = coder->env;
	enqueue_coder(env, coder);
	while (!can_take_dongles(coder) && !check_simulation_end(env))
	{
		deadline = next_wake_deadline(env, left, right);
		ts.tv_sec = deadline / 1000;
		ts.tv_nsec = (deadline % 1000) * 1000000;
		pthread_cond_timedwait(&env->cond_coders[coder->id],
			&env->arbitrator_mutex, &ts);
	}
	if (check_simulation_end(env))
	{
		dequeue_coder(env, coder);
		return (0);
	}
	env->dongle_taken[left] = 1;
	env->dongle_taken[right] = 1;
	dequeue_coder(env, coder);
	return (1);
}

int	take_dongles(t_coder *coder)
{
	t_env	*env;
	int		left;
	int		right;
	int		got_dongles;

	env = coder->env;
	if (check_simulation_end(env))
		return (0);
	left = coder->id;
	right = (coder->id + 1) % env->num_coders;
	pthread_mutex_lock(&env->arbitrator_mutex);
	got_dongles = wait_in_queue(coder, left, right);
	pthread_mutex_unlock(&env->arbitrator_mutex);
	if (got_dongles)
	{
		print_status(coder, DONGLE_CLR, "has taken a dongle");
		print_status(coder, DONGLE_CLR, "has taken a dongle");
	}
	return (got_dongles);
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
