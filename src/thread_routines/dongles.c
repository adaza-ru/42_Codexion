/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 02:30:11 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/09/06 19:58:12 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

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

int	take_dongles(t_coder *coder)
{
	if (!wait_for_turn(coder->env, coder))
		return (0);
	print_status(coder, DONGLE_CLR, "has taken a dongle");
	print_status(coder, DONGLE_CLR, "has taken a dongle");
	return (1);
}
