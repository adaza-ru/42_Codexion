/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_logic.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 02:30:11 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/09/09 02:02:18 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

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

int	can_take_dongles(t_env *env, t_coder *coder, int left, int right)
{
	int	i;
	int	other_id;

	i = 0;
	other_id = 0;
	if (!dongles_available(env, left, right))
		return (0);
	while (i < env->heap_size)
	{
		other_id = env->heap[i];
		if ((other_id == left || other_id == right
				|| (other_id + 1) % env->num_coders == left
				|| (other_id + 1) % env->num_coders == right)
			&& has_higher_priority(env, other_id, coder->id))
			return (0);
		i++;
	}
	return (1);
}
