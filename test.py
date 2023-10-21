import matplotlib.pyplot as plt
import numpy as np

if __name__ == '__main__':
    roots = [
        np.array([1.0, 0.0]),
        np.array([-0.5, np.sqrt(3.0)/2.0]),
        np.array([-0.5, -np.sqrt(3.0)/2.0])
    ]
    
    colors = ['blue', 'green', 'red']
    
    n = 100
    x = np.linspace(-2.0, 2.0, n)
    
    plt.figure()
    plt.title(r'Newton-Raphson for $z^3 - 1 = 0$')
    plt.xlabel(r'$\mathsf{Re}$')
    plt.ylabel(r'$\mathsf{Im}$', rotation=0)
    for xv in x:
        for yv in x:
            
            w = np.array([xv, yv])
            for i in range(100):
                diag = w[0]*w[0] - w[1]*w[1]
                off = 2.0*w[0]*w[1]
                p = np.array([w[0]**3 - 3.0*w[0]*w[1]**2 - 1.0, (3.0*w[0]**2 - w[1]**2)*w[1]])
                frac = w[0]**2 + w[1]**2
                frac *= frac
                frac = 1.0 / (3.0 * frac)
                jac = frac * np.array([[diag, off], [-off, diag]])
                w = w - np.dot(jac, p)
            
            index = np.argmin([np.linalg.norm(w - r) for r in roots])
            plt.scatter(xv, yv, color=colors[index])
    plt.show()
    plt.close()
