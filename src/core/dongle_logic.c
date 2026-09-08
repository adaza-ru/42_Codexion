/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_logic.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 02:30:11 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/09/09 01:08:27 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

/*
static int	is_doomed(t_env *env, t_coder *coder, int ahead)
{
	size_t	time_lived;
	size_t	time_left;
	size_t	time_needed;
	size_t	margin;

	time_lived = get_current_time() - coder->last_compile_start;
	if (time_lived >= env->time_to_burnout)
		return (1);
	time_left = env->time_to_burnout - time_lived;
	margin = env->time_to_burnout / 10;
	if (margin > 15)
		margin = 15;
	if (time_left <= margin)
		return (1);
	time_needed = (ahead) * (env->time_to_compile + env->dongle_cooldown);
	if (time_needed >= (time_left - margin))
		return (1);
	return (0);
}
*/

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

static int	edf_logic(t_env *env, t_coder *coder, int left, int right)
{
    int	i;
    int	other_id;

    i = 0;
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

static int	fifo_logic(t_env *env, t_coder *coder, int left, int right)
{
    int	i;
    int	other_id;

    i = 0;
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

int	can_take_dongles(t_env *env, t_coder *coder, int left, int right)
{
	if (!dongles_available(env, left, right))
		return (0);
	if (env->scheduler == SCH_FIFO)
		return (fifo_logic(env, coder, left, right));
	else
		return (edf_logic(env, coder, left, right));
//		return (edf_logic(env, coder));
}

/*
static int	edf_logic(t_env *env, t_coder *coder)
{
	int		i;
	int		other_id;
	int		ahead;

	i = 0;
	ahead = 0;
	while (i < env->heap_size)
	{
		other_id = env->heap[i];
		if (has_higher_priority(env, other_id, coder->id))
			ahead++;
		i++;
	}
	if (ahead != 0 && !is_doomed(env, coder, ahead))
		return (0);
	return (1);
	
}

 
static int	edf_logic(t_env *env, t_coder *coder, int left, int right)
{
	int		i;
	int		other_id;
	int		ahead;
 
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
			if (!is_doomed(env, coder, ahead))
				return (0);
			ahead++;
		}
		i++;
	}
	return (1);
}

static int	edf_logic(t_env *env, t_coder *coder, int left, int right)
{
	int		i;
	int		other_id;
	int		ahead;

	i = 0;
	ahead = 0;
	while (i < env->heap_size)
	{
		other_id = env->heap[i];
		if ((other_id == left || other_id == right
				|| (other_id + 1) % env->num_coders == left
				|| (other_id + 1) % env->num_coders == right)
			&& has_higher_priority(env, other_id, coder->id))
			ahead++;
		i++;
	}
	if (ahead != 0 && !is_doomed(env, coder, ahead))
		return (0);
	return (1);
}

*/