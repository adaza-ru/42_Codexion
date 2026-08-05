/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/03 20:46:30 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/08/05 02:46:46 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

size_t	get_current_time(void)
{
	struct timeval	time;

	if (gettimeofday(&time, NULL) == -1)
	{
		fprintf(stderr, DEATH_CLR "\n\n¡¡¡UNEXPECTED ERROR: gettimeofday "
			"failed!!! The simulation is corrupted. Please restart.\n\n" RESET);
		return (0);
	}
	return ((time.tv_sec * 1000) + (time.tv_usec / 1000));
}

void	codex_usleep(size_t milliseconds, t_env *env)
{
	size_t	start;

	start = get_current_time();
	while ((get_current_time() - start) < milliseconds)
	{
		pthread_mutex_lock(&env->end_mutex);
		if (env->simulation_end)
		{
			pthread_mutex_unlock(&env->end_mutex);
			break ;
		}
		pthread_mutex_unlock(&env->end_mutex);
		usleep(500);
	}
}
