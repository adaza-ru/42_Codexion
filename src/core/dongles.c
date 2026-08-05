/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 02:30:11 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/08/05 02:30:49 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

void	take_dongles(t_coder *coder)
{
	t_env	*env;

	env = coder->env;
	// 1. Entramos en la sala del Árbitro (Nadie más puede modificar recursos)
	pthread_mutex_lock(&env->arbitrator_mutex);

	// 2. Nos apuntamos en la lista de espera (FIFO o EDF)
	enqueue_coder(env, coder);

	// 3. El bucle de espera (Control de sueño)
	while (!can_take_dongles(env, coder))
	{
		// Si no podemos cogerlos, soltamos el Árbitro y nos dormimos.
		// Al despertar, el sistema volverá a bloquear el Árbitro automáticamente.
		pthread_cond_wait(&env->cond_coders[coder->id], &env->arbitrator_mutex);
	}

	// 4. ¡Nos toca! Ya podemos coger los dongles.
	// Nos borramos de la lista de espera.
	dequeue_coder(env, coder);

	// 5. Marcamos los dongles físicos como "Ocupados"
	grab_dongles_state(env, coder);

	// 6. Salimos de la sala del Árbitro
	pthread_mutex_unlock(&env->arbitrator_mutex);
}