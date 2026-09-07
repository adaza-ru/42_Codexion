/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/28 18:26:06 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/09/05 19:24:05 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

int	main(int argc, char **argv)
{
	int		error_mark;
	t_env	env;

	error_mark = check_arg_errors(argc, argv);
	if (!error_mark)
		memset(&env, 0, sizeof(t_env));
		error_mark = init_env(&env, argv);
	if (!error_mark)
		error_mark = start_simulation(&env);
	if (error_mark)
	{
		print_error(error_mark);
		return (error_mark);
	}
	clean_up_everything(&env);
	return (error_mark);
}
